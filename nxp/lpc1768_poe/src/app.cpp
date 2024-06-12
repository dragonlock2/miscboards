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

    // TODO ethernet, lwIP

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
