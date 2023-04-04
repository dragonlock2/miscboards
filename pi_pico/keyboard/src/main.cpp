#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <tusb.h>
#include "config.h"
#include "kscan.h"
#include "encoder.h"
#include "usb.h"
#include "macro.h"
#include "gui.h"

// large shared objects
static kscan   keys(NUM_ROWS, NUM_COLS, ROW_PINS, COL_PINS);
static encoder enc(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_REVERSE);
static USB     usb;
static BLE     ble;
static macro   mac(keys, KEYMAP, KEYMODMAP, ENCMAP, CONSUMER_KEYMAP, CONSUMER_ENCMAP);

static ssd1306 oled(OLED_HEIGHT, OLED_WIDTH, OLED_I2C, OLED_SDA, OLED_SCL, OLED_ADDR, OLED_FREQ);
static ws2812b leds(NUM_ROWS, NUM_COLS, LED_PINS);
static GUI     gui(keys, usb, ble, oled, leds, SLEEP_PIN);

// ticker
static volatile uint64_t cpu0_time, cpu1_time;
static volatile bool cpu0_tick, cpu1_tick;
static bool ticker(repeating_timer_t *rt) {
    // BLE messes with timing
    cpu0_tick = true;
    cpu1_tick = true;
    return true;
}

// threads
static void cpu0_thread(void) {
    int ticks = 0;

    while (true) {
        while (!cpu0_tick);
        uint64_t s = time_us_64();

        keys.scan();

        int diff = enc;
        if (diff != 0 && multicore_fifo_wready()) {
            multicore_fifo_push_blocking((uint32_t) diff);
        }
        ticks += diff;

        hid_keyboard_report_t kb_report = {0};
        hid_mouse_report_t mouse_report = {0};
        uint16_t consumer_report = 0;
        mac.get_report(ticks, kb_report, mouse_report, consumer_report);

        usb.process();
        usb.set_report(kb_report, mouse_report, consumer_report);

        ble.process();
        ble.set_report(kb_report, mouse_report, consumer_report);

        cpu0_time = time_us_64() - s;
        cpu0_tick = false;
    }
}

static void cpu1_thread(void) {
    while (true) {
        while (!cpu1_tick);
        uint64_t s = time_us_64();

        gui.process(cpu0_time, cpu1_time);
        gui.display();

        cpu1_time = time_us_64() - s;
        cpu1_tick = false;
    }
}

int main() {
    repeating_timer_t timer;
    add_repeating_timer_ms(1, ticker, NULL, &timer);

    multicore_launch_core1(cpu1_thread);
    cpu0_thread();

    // TODO BLE LEDs
    // TODO debug hang during BLE pairing process
        // increase cpu1 tick rate => much more likely to fail
        // sometimes only stalls cpu1
        // completely fine if can get thru pairing process
}
