#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <tusb.h>
#include "config.h"
#include "kscan.h"
#include "encoder.h"
#include "usb.h"
#include "gui.h"

// large shared objects
static kscan   keys(NUM_ROWS, NUM_COLS, ROW_PINS, COL_PINS);
static encoder enc(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_REVERSE);
static USB     usb;

static ssd1306 oled(OLED_HEIGHT, OLED_WIDTH, OLED_I2C, OLED_SDA, OLED_SCL, OLED_ADDR, OLED_FREQ);
static ws2812b leds(NUM_ROWS, NUM_COLS, LED_PINS);
static GUI     gui(keys, oled, leds, SLEEP_PIN);

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
    int ticks = 0;

    while (true) {
        while (!cpu0_tick);
        keys.scan();

        int diff = enc;
        if (diff != 0 && multicore_fifo_wready()) {
            multicore_fifo_push_blocking((uint32_t) diff);
        }
        ticks += diff;

        // TODO cpu0 - BLE, USB
            // add a macro class which maps keys/enc to report
            // add a BLE and USB classes which can set report
        hid_keyboard_report_t kb_report = {0};
        hid_mouse_report_t mouse_report = {0};

        // TODO remove
        if (keys(0, 0)) {
            kb_report.keycode[0] = HID_KEY_A;
        }
        if (keys(0, 1)) {
            mouse_report.x = 1;
        }

        usb.process();
        usb.set_report(kb_report, mouse_report);

        cpu0_tick = false;
    }
}

static void cpu1_thread(void) {
    while (true) {
        while (!cpu1_tick);
        // TODO cpu1 - GUI (add CPU usage)
        gui.process(usb.connected());
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
