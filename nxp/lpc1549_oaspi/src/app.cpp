#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "usb.h"
#include "usr.h"

extern "C" void app_main(void*) {
    SPI spi;
    eth::OASPI oaspi(spi, usr::phyrst);
    eth::Eth eth(oaspi, usr::phyint);
    USB usb(oaspi, eth);
    printf("booted! 0x%02x\r\n", usr::id());

    bool t = 0, r = 0, g = 0, b = 1;
    while (true) {
        usr::rgb(r, g, b);
        t = r; r = g; g = b; b = t;
        auto [txu, rxu] = usb.get_error();
        auto [txe, rxe] = eth.get_error();
        printf("time: 0x%08lx tx_drop: %ld rx_drop: %ld\r\n", xTaskGetTickCount(), txu + txe, rxu + rxe);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(nullptr);
}
