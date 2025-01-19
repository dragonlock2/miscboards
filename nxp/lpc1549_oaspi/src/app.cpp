#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "usb.h"
#include "usr.h"

extern "C" void app_main(void*) {
    SPI spi;
#ifdef CONFIG_ADIN1110
    eth::OASPI_ADIN1110 oaspi(spi, usr::phyrst);
#elifdef CONFIG_NCN26010
    eth::OASPI_NCN26010 oaspi(spi, usr::phyrst);
#else
    #error "define MACPHY pls"
#endif
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
