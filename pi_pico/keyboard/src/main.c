#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include "kscan.h"

int main() {
    cyw43_arch_init();
    stdio_init_all();
    kscan_init();

    bool prev[KSCAN_ROWS][KSCAN_COLS] = {0};
    while (true) {
        kscan_run1ms();

        for (uint i = 0; i < KSCAN_ROWS; i++) {
            for (uint j = 0; j < KSCAN_COLS; j++) {
                bool c = kscan_read(i, j);
                if (c && !prev[i][j]) {
                    printf("%d, %d pressed\r\n", i, j);
                } else if (!c && prev[i][j]) {
                    printf("%d, %d released\r\n", i, j);
                }
                prev[i][j] = c;
            }
        }
        sleep_ms(1);
    }

    // TODO cpu0 - BLE, USB, kscan, encoder
    // TODO cpu1 - LEDs @ 1kHz, SSD1306 @ 60FPS
}
