#include <FreeRTOS.h>
#include <task.h>
#include <ch32v20x.h>

extern void freertos_risc_v_trap_handler(void);
extern void app_main(void *args);

static void timer_handler(void) {
    SysTick->SR = 0;
    portYIELD_FROM_ISR(xTaskIncrementTick());
}

void freertos_risc_v_application_exception_handler(uint32_t mcause) {
    extern void (*vectors[])(void);
    vectors[mcause]();
}

void freertos_risc_v_application_interrupt_handler(uint32_t mcause) {
    extern void (*vectors[])(void);
    vectors[mcause & 0x7FFFFFFF]();
}

void vPortSetupTimerInterrupt(void) {
    NVIC_SetVector(SysTicK_IRQn, timer_handler);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = SystemCoreClock / configTICK_RATE_HZ - 1;
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void) xTask;
    printf("%s overflow\r\n", pcTaskName);
    __disable_irq();
    while (1);
}

void *malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void *ptr) {
    vPortFree(ptr);
}

void *_malloc_r(struct _reent *r, size_t size) {
    (void) r;
    return malloc(size);
}

void _free_r(struct _reent *r, void *ptr) {
    (void) r;
    return free(ptr);
}

int main(void) {
    // disable interrupt nesting, FreeRTOS lacks support
    asm volatile (
        "li t0, 0x0     \n"
        "csrw 0x804, t0 \n" // INTSYSCR
        : : : "t0"
    );

    // hook in FreeRTOS interrupt/exception handler
    static void (*trap_handler)(void) __attribute__((aligned(4))) = freertos_risc_v_trap_handler;
    asm volatile (
        "mv t0, %0         \n"
        "li t1, 0xfffffffc \n"
        "and t0, t0, t1    \n"
        "ori t0, t0, 0x2   \n"
        "csrw mtvec, t0    \n"
        : : "r" (&trap_handler) : "t0", "t1"
    );

    // create app task
    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE, NULL, configAPP_MAIN_PRIORITY, NULL);

    vTaskStartScheduler();
    while (1);
}
