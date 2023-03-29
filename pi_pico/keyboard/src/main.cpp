#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include "config.h"
#include "kscan.h"
#include "encoder.h"
#include "gui.h"

// large shared objects
kscan   keys(NUM_ROWS, NUM_COLS, ROW_PINS, COL_PINS);
encoder enc(ENCODER_PIN_A, ENCODER_PIN_B);
ssd1306 oled(OLED_HEIGHT, OLED_WIDTH, OLED_I2C, OLED_SDA, OLED_SCL, OLED_ADDR, OLED_FREQ);
ws2812b leds(NUM_ROWS, NUM_COLS, LED_PINS);
GUI     gui(keys, oled, leds, SLEEP_PIN);

// ticker
static volatile bool cpu0_tick, cpu1_tick;
static bool ticker(repeating_timer_t *rt) {
    if (cpu0_tick || cpu1_tick) {
        panic("overrun, something took too long");
    }
    cpu0_tick = true;
    cpu1_tick = true;
    return true;
}

// threads
static void cpu0_thread(void) {
    while (true) {
        while (!cpu0_tick);
        // TODO cpu0 - BLE, USB, encoder
        keys.scan();
        enc.scan();

        printf("%d\r\n", static_cast<int>(enc));

        cpu0_tick = false;
    }
}

static void cpu1_thread(void) {
    while (true) {
        while (!cpu1_tick);
        // TODO cpu1 - GUI
        gui.process();
        gui.display();

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
