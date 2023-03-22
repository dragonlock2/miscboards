#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include "kscan.h"
#include "ws2812b.h"

static volatile bool cpu0_tick, cpu1_tick;
static bool ticker(repeating_timer_t *rt) {
    cpu0_tick = true;
    cpu1_tick = true;
    return true;
}

static void cpu0_thread(void) {
    while (true) {
        while (!cpu0_tick);
        // TODO cpu0 - BLE, USB, kscan, encoder
        kscan_run1ms();

        cpu0_tick = false;
    }
}

static void cpu1_thread(void) {
    while (true) {
        while (!cpu1_tick);
        // TODO cpu1 - LEDs @ 1kHz, SSD1306 @ 60FPS, sleep req

        cpu1_tick = false;
    }
}

int main() {
    cyw43_arch_init();
    stdio_init_all();
    kscan_init();
    ws2812b_init();

    repeating_timer_t timer;
    add_repeating_timer_ms(1, ticker, NULL, &timer);

    multicore_launch_core1(cpu1_thread);
    cpu0_thread();
}
