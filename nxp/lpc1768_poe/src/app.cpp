#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include "eth.h"
#include "usb.h"
#include "usr.h"

extern "C" void app_main(void*) {
    eth_init();
    usb_init();
    printf("booted!\r\n");

    // TODO FreeRTOS coreHTTP

    while (1) {
        usr_rgb(1, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        usr_rgb(0, 1, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        usr_rgb(0, 0, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}
