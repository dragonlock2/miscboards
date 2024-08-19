#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "eth.h"
#include "oaspi.h"
#include "spi.h"
#include "usb.h"
#include "usr.h"

#if defined(CONFIG_ADIN1110) && defined(CONFIG_NCN26010)
#error "define one config only"
#endif

#if !defined(CONFIG_ADIN1110) && !defined(CONFIG_NCN26010)
#error "define one config pls"
#endif

uint8_t tud_network_mac_address[6] = {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00};

extern "C" void app_main(void*) {
    std::array<uint32_t, 4> uid;
    Chip_IAP_ReadUID(&uid[0]);
    uint8_t mac = 0xFF;
    for (int i = 0; i < 16; i++) {
        mac ^= reinterpret_cast<uint8_t*>(&uid[0])[i]; // randomize mac
    }
    tud_network_mac_address[5] = mac;

    spi_init();
    oaspi_init();
    eth_init();
    usb_init();
    printf("booted! 0x%02x\r\n", mac);

    bool t = 0, r = 0, g = 0, b = 1;
    while (true) {
        usr_rgb(r, g, b);
        t = r; r = g; g = b; b = t;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    vTaskDelete(NULL);
}
