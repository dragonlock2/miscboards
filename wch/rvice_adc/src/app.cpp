#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "btn.h"
#include "fpga.h"
#include "rgb.h"
#include "usb.h"

extern "C" void app_main(void *args) {
    btn_init();
    fpga_init();
    rgb_init();

    xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, NULL,
        configMAX_PRIORITIES - 1, NULL);

    printf("booted! %p\r\n", args);

    TickType_t wait = xTaskGetTickCount();
    while (true) {
        static bool i;
        rgb_write(i, btn_read(), fpga_booted());
        i = !i;

        vTaskDelayUntil(&wait, 500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);

    // TODO sync to own clock, not usb clock, for audio
    // TODO test 2 basic spis, do a basic byte delay
    // TODO spi with dma, basic stream, tbh it's so slow might be able to do polling
    // TODO adc stream
}
