#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>

extern "C" void app_main(void*) {
    printf("booted!\r\n");

    // TODO led, btn
    // TODO mdio
    // TODO extra conn
    // TODO usb
    // TODO ethernet

    while (1) {
        printf("time %ld\r\n", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}
