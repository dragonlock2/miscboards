#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>

#ifdef CYW43_WL_GPIO_LED_PIN
#include <pico/cyw43_arch.h>
#endif

extern void app_main(void*);

void app_init(void) {
    stdio_init_all();

#if LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) { vTaskDelay(1); }
#endif

#ifdef CYW43_WL_GPIO_LED_PIN
    configASSERT(cyw43_arch_init() == 0);
#endif
}

void app_trampoline(void*) {
    // init must run on one core
    vTaskCoreAffinitySet(NULL, 0b01);
    app_init();
    vTaskCoreAffinitySet(NULL, 0b11);

    app_main(NULL);
}

void *malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void *ptr) {
    vPortFree(ptr);
}

int main(void) {
    xTaskCreate(app_trampoline, "app_main", configAPP_MAIN_STACK_SIZE, NULL, configAPP_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
    while (1);
}
