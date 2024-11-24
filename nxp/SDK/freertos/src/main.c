#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void app_main(void*);

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

    xTaskCreate(app_main, "app_main", configAPP_MAIN_STACK_SIZE, NULL, configAPP_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
    while (1);
}
