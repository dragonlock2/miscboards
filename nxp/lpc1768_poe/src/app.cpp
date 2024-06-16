#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include "eth.h"
#include "usb.h"
#include "usr.h"

static void eth_tx_task(void*) {
    uint32_t cnt = 0;
    bool prev_btn = false;
    while (true) {
        bool btn = usr_btn();
        if (eth_link_up() && btn && !prev_btn) {
            uint8_t data[69 + (cnt % 16)];
            for (size_t i = 0; i < sizeof(data); i++) {
                data[i] = i;
            }
            memcpy(&data[14], &cnt, 4);
            eth_write(data, sizeof(data));
            cnt++;
        }
        prev_btn = btn;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void app_main(void*) {
    eth_init();
    usb_init();
    printf("booted!\r\n");

    // TODO lwIP - mdns, httpd
    xTaskCreate(eth_tx_task, "eth_tx_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    while (1) {
        if (eth_link_up()) {
            usr_rgb(0, 1, 0);
        } else {
            usr_rgb(1, 0, 0);
        }

        uint8_t *data;
        size_t len;
        while (eth_read(data, len)) {
            // def some overruns at this speed
            printf("rx src: %02x:%02x:%02x:%02x:%02x:%02x dst: %02x:%02x:%02x:%02x:%02x:%02x len: %d\r\n",
                data[6], data[7], data[8], data[9], data[10], data[11],
                data[0], data[1], data[2], data[3], data[4],  data[5],
                len);
            eth_read_done();
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
    vTaskDelete(NULL);
}
