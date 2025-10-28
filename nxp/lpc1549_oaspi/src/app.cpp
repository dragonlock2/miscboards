#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "usb.h"
#include "usr.h"

void app_main(void*) {
    SPI spi;
#ifdef CONFIG_ADIN1110
    eth::OASPI_ADIN1110 oaspi(spi, usr::phyrst);
#elifdef CONFIG_NCN26010
    eth::OASPI_NCN26010 oaspi(spi, usr::phyrst);
#else
    #error "define MACPHY pls"
#endif
    static eth::Eth eth(oaspi, usr::phyint); // too big for stack
    USB usb(oaspi, eth);
    std::printf("booted! 0x%02x\r\n", usr::id());

    bool tx_state = false, rx_state = false;
    while (true) {
        auto [tx, rx] = eth.get_packets();
        tx_state = tx_state ? false : (tx > 0);
        rx_state = rx_state ? false : (rx > 0);

        bool r = eth.error();
        bool g = rx_state;
        bool b = tx_state;
        usr::rgb(r, g, b);
        vTaskDelay(pdMS_TO_TICKS(25));
    }
    vTaskDelete(nullptr);
}
