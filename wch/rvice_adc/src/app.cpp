#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "btn.h"
#include "fpga.h"
#include "rgb.h"

extern "C" void app_main(void *args) {
    (void) args;
    btn_init();
    fpga_init();
    printf("booted!\r\n");

    TickType_t wait = xTaskGetTickCount();
    bool i = false;
    while (true) {
        printf("%ld\r\n", wait);
        rgb_write(i, 0, fpga_booted());
        i = !i;

        vTaskDelayUntil(&wait, 500 / portTICK_PERIOD_MS);
    }

    // TODO rewrite in cpp fashion esp for inits, templates!
    // TODO tinyusb? need to get usb bulk transfers working
    // TODO test 2 basic spis, do a basic byte delay
}
