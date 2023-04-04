#include <cstdio>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <tusb.h>
#include "gui.h"

#define SLEEP_TIME  (30000) // ms
#define SLEEP_PULSE (100) // ms (>50ms pulse => sleep)

GUI::GUI(kscan& keys, USB& usb, BLE& ble, ssd1306& oled, ws2812b& leds, uint sleep)
:
    keys(keys), usb(usb), ble(ble), oled(oled), leds(leds), sleep(sleep), sleep_ctr(SLEEP_TIME), frame_ctr(0)
{
    gpio_init(sleep);
    gpio_set_dir(sleep, true);
    gpio_put(sleep, 0);
}

void GUI::process(uint64_t cpu0_time, uint64_t cpu1_time) {
    // receive encoder diff from cpu0 FIFO
    int enc = 0;
    while (multicore_fifo_rvalid()) {
        enc += (int) multicore_fifo_pop_blocking();
    }

    // sleep check
    sleep_ctr = (usb.connected() || ble.connected() || sleep_check(enc)) ? SLEEP_TIME : sleep_ctr - 1;
    gpio_put(sleep, !(sleep_ctr == 0 || sleep_ctr > SLEEP_PULSE));

    // render GUI
    frame_ctr++;

    // release causes fade
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                leds(i, j) = ws2812b_color(0, 0, 100);
            } else {
                ws2812b_color &c = leds(i, j);
                if (frame_ctr % 5 == 0) {
                    if (c.r > 0) { c.r -= 1; }
                    if (c.g > 0) { c.g -= 1; }
                    if (c.b > 0) { c.b -= 1; }
                }
            }
        }
    }

    // provide info
    enc_ticks += enc;
    uint32_t key;
    uint y = 0;

    oled.clear();
    oled.set_cursor(0, y);
    oled_printf("sleep countdown: %ds", sleep_ctr / 1000);
    y += 8;
    
    if (frame_ctr % 100 == 0) {
        this->cpu0_time = cpu0_time;
        this->cpu1_time = cpu1_time;
    }

    oled.set_cursor(0, y);
    oled_printf("cpu0 %4lldus, cpu1 %4lldus", this->cpu0_time, this->cpu1_time);
    y += 8;

    oled.set_cursor(0, y);
    oled_printf("encoder ticks: %d", enc_ticks);
    y += 8;

    oled.set_cursor(0, y);
    if (usb.connected()) {
        oled_printf("usb connected");
        y += 8;
    }

    oled.set_cursor(0, y);
    if (ble.connected()) {
        oled_printf("ble connected");
        y += 8;
    }

    oled.set_cursor(0, y);
    if (ble.passkey(key)) {
        oled_printf("passkey: %ld", key);
        y += 8;
    }

    oled.set_cursor(0, y);
    if ((usb.connected() && (usb.led_status() & KEYBOARD_LED_CAPSLOCK)) ||
        (ble.connected() && (ble.led_status() & KEYBOARD_LED_CAPSLOCK))) {
        oled_printf("caps locked!");
        y += 8;
    }
}

void GUI::display(void) {
    leds.display();
    oled.display();
}

void GUI::oled_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char s[64];
    vsnprintf(s, sizeof s, format, args);
    oled.draw_string(s);
    va_end(args);
}

bool GUI::sleep_check(int enc) {
    if (enc != 0) {
        return true;
    }
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                return true;
            }
        }
    }
    return false;
}
