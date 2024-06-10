#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void app_main(void*);

__attribute__((used))
void __malloc_lock(struct _reent *r) {
    (void) r;
    vTaskSuspendAll();
}

__attribute__((used))
void __malloc_unlock(struct _reent *r) {
    (void) r;
    xTaskResumeAll();
}

int main(void) {
    NVIC_SetVector(SVCall_IRQn,  (uint32_t) vPortSVCHandler);
    NVIC_SetVector(PendSV_IRQn,  (uint32_t) xPortPendSVHandler);
    NVIC_SetVector(SysTick_IRQn, (uint32_t) xPortSysTickHandler);

    xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE, NULL, configAPP_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
    while (1);
}
