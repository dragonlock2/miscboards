#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include "kscan.h"
#include "ws2812b.h"

static volatile bool cpu0_tick, cpu1_tick;
static bool ticker(repeating_timer_t *rt) {
    cpu0_tick = true;
    cpu1_tick = true;
    return true;
}

int main() {
    cyw43_arch_init();
    stdio_init_all();
    kscan_init();
    ws2812b_init();

    repeating_timer_t timer;
    add_repeating_timer_ms(1, ticker, NULL, &timer);

    bool prev[KSCAN_ROWS][KSCAN_COLS] = {0};
    while (true) {
        while (!cpu0_tick);
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

        cpu0_tick = false;
    }

    // TODO cpu0 - BLE, USB, kscan, encoder
    // TODO cpu1 - LEDs @ 1kHz, SSD1306 @ 60FPS, sleep req
}
