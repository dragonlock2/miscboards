#include <array>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

extern void app_main(void*);

extern "C" {

extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

__attribute__((used))
void *_malloc_r(struct _reent*, size_t size) {
    return pvPortMalloc(size);
}

__attribute__((used))
void _free_r(struct _reent*, void *ptr) {
    vPortFree(ptr);
}

int main(void) {
    NVIC_SetVector(SVCall_IRQn,  (uint32_t) vPortSVCHandler);
    NVIC_SetVector(PendSV_IRQn,  (uint32_t) xPortPendSVHandler);
    NVIC_SetVector(SysTick_IRQn, (uint32_t) xPortSysTickHandler);

    static StaticTask_t task_buffer;
    static std::array<StackType_t, configAPP_MAIN_STACK_SIZE> task_stack;
    configASSERT(xTaskCreateStatic(app_main, "app_main", task_stack.size(),
        nullptr, configAPP_MAIN_PRIORITY, task_stack.data(), &task_buffer));
    vTaskStartScheduler();
    while (1);
}

}
