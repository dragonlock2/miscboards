#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "eth.h"
#include "oaspi.h"
#include "spi.h"
#include "usb.h"
#include "usr.h"

uint8_t tud_network_mac_address[6] = {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00};

extern "C" void app_main(void*) {
    usb_init();
    spi_init();
    oaspi_init();
    eth_init();
    printf("booted!\r\n");

    // TODO usb rndis/ecm, randomize the mac addr

    bool c = 0;
    while (true) {
        c = !c;
        usr_rgb(c, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}
