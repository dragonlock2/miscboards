#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include "audio.h"
#include "btn.h"
#include "fpga.h"
#include "rgb.h"
#include "usb.h"

extern "C" void app_main(void *args) {
    (void) args;
    btn_init();
    fpga_init();
    rgb_init();
    usb_init();
    audio_init();
    printf("booted!\r\n");

    TickType_t wait = xTaskGetTickCount();
    bool i = false;
    while (true) {
        rgb_write(i, btn_read(), fpga_booted());
        i = !i;

        vTaskDelayUntil(&wait, pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);

    // TODO test 2 basic spis, do a basic byte delay
    // TODO spi with dma, basic stream
    // TODO adc stream
}
