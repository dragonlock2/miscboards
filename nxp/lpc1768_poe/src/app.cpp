#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "eth.h"
#include "usr.h"

extern "C" void app_main(void*) {
    eth_init();
    printf("booted!\r\n");

    // TODO extra conn
    // TODO usb
    // TODO ethernet

    while (1) {
        if (eth_mdio_read(0x01) & (1 << 2)) {
            usr_rgb(0, 1, 0);
        } else {
            usr_rgb(1, 0, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}
