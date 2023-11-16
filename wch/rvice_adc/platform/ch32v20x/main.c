#include <FreeRTOS.h>
#include <task.h>
#include <ch32v20x.h>
#include <ch32v20x_rcc.h>

extern void freertos_risc_v_trap_handler(void);
extern void dbg_init(void);
extern void rgb_init(void);
extern void app_main(void *args);

__attribute__((interrupt))
static void timer_handler(void) {
    SysTick->SR = 0;
    portYIELD_FROM_ISR(xTaskIncrementTick());
}

void freertos_risc_v_application_exception_handler(uint32_t mcause) {
    extern void (*vectors[])(void);
    vectors[mcause & 0x7FFFFFFF]();
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

void *malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void *ptr) {
    vPortFree(ptr);
}

static void dummy(void) {}

int main(void) {
    // hook in FreeRTOS handler
    static void (*trap_handler)(void) __attribute__((aligned(4))) = freertos_risc_v_trap_handler;
    asm volatile (
        "mv t0, %0         \n"
        "li t1, 0xfffffffc \n"
        "and t0, t0, t1    \n"
        "ori t0, t0, 0x2   \n"
        "csrw mtvec, t0    \n"
        : : "r" (&trap_handler) : "t0", "t1"
    );

    // TODO ecall gets called twice? stuck task switching
    // timer_handler never gets called more than a few times, probs stuck in isr?
    // that's probs why they pend a software irq
    NVIC_SetVector(Ecall_M_Mode_IRQn, dummy);

    // configure clocks
    SetSysClockTo144_HSE();
    configASSERT(SystemCoreClock == configCPU_CLOCK_HZ);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,  ENABLE);

    // init debugging early
    dbg_init();
    rgb_init();

    // create app task
    xTaskCreate(
        app_main,
        "app_main",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
    );

    vTaskStartScheduler();
    while (1);
}
