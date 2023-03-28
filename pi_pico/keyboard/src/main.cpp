#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include "config.h"
#include "kscan.h"
#include "ws2812b.h"
#include "ssd1306.h"

// shared data
kscan keys(NUM_ROWS, NUM_COLS, ROW_PINS, COL_PINS);

// ticker
static volatile bool cpu0_tick, cpu1_tick;
static bool ticker(repeating_timer_t *rt) {
    if (cpu0_tick || cpu1_tick) {
        // panic("overrun, something took too long"); // TODO enable once all done
    }
    cpu0_tick = true;
    cpu1_tick = true;
    return true;
}

// threads
static void cpu0_thread(void) {
    while (true) {
        while (!cpu0_tick);
        // TODO cpu0 - BLE, USB, kscan, encoder
        keys.scan();

        static bool prev[NUM_ROWS][NUM_COLS];
        for (uint i = 0; i < NUM_ROWS; i++) {
            for (uint j = 0; j < NUM_COLS; j++) {
                bool c = keys(i, j);
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
}

static void cpu1_thread(void) {
    ssd1306 oled(OLED_HEIGHT, OLED_WIDTH, OLED_I2C, OLED_SDA, OLED_SCL, OLED_ADDR);
    ws2812b leds(NUM_ROWS, NUM_COLS, LED_PINS);

    while (true) {
        while (!cpu1_tick);
        // TODO cpu1 - LEDs @ 1000FPS, SSD1306 @ 125FPS, sleep req (no keys pressed 30s), add GUI
        for (uint i = 0; i < NUM_ROWS; i++) {
            for (uint j = 0; j < NUM_COLS; j++) {
                leds(i, j) = keys(i, j) ? ws2812b_color(0, 40, 0) : ws2812b_color(40, 0, 0);
            }
        }
        static char c = 0;
        oled.clear();
        oled.set_cursor(0, 0);
        oled.draw_char(c++);

        leds.display();
        oled.display(); // TODO check FPS

        cpu1_tick = false;
    }
}

int main() {
    stdio_init_all();
    cyw43_arch_init();

    repeating_timer_t timer;
    add_repeating_timer_ms(1, ticker, NULL, &timer);

    multicore_launch_core1(cpu1_thread);
    cpu0_thread();
}
