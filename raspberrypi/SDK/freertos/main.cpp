#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>

#ifdef CYW43_WL_GPIO_LED_PIN
#include <pico/cyw43_arch.h>
#endif

extern void app_main(void*);

static void app_init(void) {
    stdio_init_all();

#if LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) { vTaskDelay(1); }
#endif

#ifdef CYW43_WL_GPIO_LED_PIN
    configASSERT(cyw43_arch_init() == 0);
#endif
}

static void app_trampoline(void*) {
#if configNUMBER_OF_CORES > 1
    // init must run on one core
    vTaskCoreAffinitySet(NULL, 0b01);
    app_init();
    vTaskCoreAffinitySet(NULL, 0b11);
#else
    app_init();
#endif

    app_main(NULL);
}

extern "C" {

void *malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void *ptr) {
    vPortFree(ptr);
}

int main(void) {
    configASSERT(xTaskCreate(app_trampoline, "app_main", configAPP_MAIN_STACK_SIZE,
        NULL, configAPP_MAIN_PRIORITY, NULL) == pdPASS);
    vTaskStartScheduler();
    while (1);
}

}
