#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "oaspi.h"
#include "spi.h"
#include "usb.h"
#include "usr.h"

extern "C" void app_main(void*) {
    usb_init();
    spi_init();
    oaspi_init();
    printf("booted!\r\n");

    // TODO interrupts
    // TODO full ethernet driver?

    bool c = 0;
    while (true) {
        c = !c;
        usr_rgb(c, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}
