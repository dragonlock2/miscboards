#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>

extern void app_main(void*);

void *malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void *ptr) {
    vPortFree(ptr);
}

int main(void) {
    stdio_init_all();
#if LIB_PICO_STDIO_USB
    while (!stdio_usb_connected());
#endif

    xTaskCreate(app_main, "app_main", configAPP_MAIN_STACK_SIZE, NULL, configAPP_MAIN_PRIORITY, NULL);
    vTaskStartScheduler();
    while (1);
}
