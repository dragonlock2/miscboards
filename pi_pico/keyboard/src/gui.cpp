#include <cstdio>
#include <pico/stdlib.h>
#include <tusb.h>
#include "gui.h"

#define SLEEP_TIME  (30000) // ms
#define SLEEP_PULSE (100) // ms (>50ms pulse => sleep)

#define ROW_HEIGHT (8) // pixels

GUI::GUI(kscan& keys, USB& usb, BLE& ble, ssd1306& oled, ws2812b& leds, uint sleep)
:
    keys(keys), usb(usb), ble(ble), oled(oled), leds(leds), sleep(sleep), sleep_ctr(SLEEP_TIME),
    enc_ticks(0), display_next(get_absolute_time()), fade_next(get_absolute_time()), cpu0_next(get_absolute_time()), cpu0_time(0)
{
    gpio_init(sleep);
    gpio_set_dir(sleep, true);
    gpio_put(sleep, 0);
}

void GUI::process(int enc, uint64_t cpu0_time) {
    // gets called every 0.6ms to 8ms due to BLE latency

    // sleep check
    bool usb_connected = usb.connected();
    bool ble_connected = ble.connected();
    sleep_ctr = (usb_connected || ble_connected || sleep_check(enc)) ? SLEEP_TIME : sleep_ctr - 1;
    gpio_put(sleep, !(sleep_ctr == 0 || sleep_ctr > SLEEP_PULSE));

    // release causes fade
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                leds(i, j) = ws2812b_color(0, 0, 100);
            } else {
                ws2812b_color &c = leds(i, j);
                if (time_reached(fade_next)) {
                    if (c.r > 0) { c.r -= 2; }
                    if (c.g > 0) { c.g -= 2; }
                    if (c.b > 0) { c.b -= 2; }
                }
            }
        }
    }
    if (time_reached(fade_next)) {
        fade_next = make_timeout_time_ms(10);
    }

    // provide info
    enc_ticks += enc;
    uint32_t key;
    uint y = 0;

    oled.clear();
    oled.set_cursor(0, y);
    oled_printf("sleep countdown: %ds", sleep_ctr / 1000);
    y += 8;
    
    if (time_reached(cpu0_next)) {
        this->cpu0_time = cpu0_time;
        cpu0_next = make_timeout_time_ms(100);
    }

    oled.set_cursor(0, y);
    oled_printf("cpu0: %4lldus", this->cpu0_time);
    y += ROW_HEIGHT;

    oled.set_cursor(0, y);
    oled_printf("encoder ticks: %d", enc_ticks);
    y += ROW_HEIGHT;

    oled.set_cursor(0, y);
    if (usb_connected && ble_connected) {
        oled_printf("usb & ble connected :P");
        y += ROW_HEIGHT;
    } else if (usb_connected) {
        oled_printf("usb connected");
        y += ROW_HEIGHT;
    } else if (ble_connected) {
        oled_printf("ble connected");
        y += ROW_HEIGHT;
    }

    oled.set_cursor(0, y);
    if (ble.passkey(key)) {
        oled_printf("passkey: %ld", key);
        y += ROW_HEIGHT;
    }

    oled.set_cursor(0, y);
    if ((usb_connected && (usb.led_status() & KEYBOARD_LED_CAPSLOCK)) ||
        (ble_connected && (ble.led_status() & KEYBOARD_LED_CAPSLOCK))) {
        oled_printf("caps locked!");
        y += ROW_HEIGHT;
    }
}

void GUI::display(void) {
    if (time_reached(display_next)) {
        display_next = make_timeout_time_ms(1);
        leds.display();
        oled.display();
    }
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
