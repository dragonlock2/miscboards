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

    // TODO spi dma
    // TODO interrupts
    // TODO full ethernet driver?

    usr_rgb(1, 0, 0);

    vTaskDelete(NULL);
}
