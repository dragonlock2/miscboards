#include <cstdio>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <tusb.h>
#include "gui.h"

#define SLEEP_TIME  (30000) // ms
#define SLEEP_PULSE (100) // ms (>50ms pulse => sleep)

GUI::GUI(kscan& keys, ssd1306& oled, ws2812b& leds, uint sleep)
:
    keys(keys), oled(oled), leds(leds), sleep(sleep), sleep_ctr(SLEEP_TIME)
{
    gpio_init(sleep);
    gpio_set_dir(sleep, true);
    gpio_put(sleep, 0);
}

void GUI::process(bool usb_connected) {
    // receive encoder diff from cpu0 FIFO
    int enc = 0;
    if (multicore_fifo_rvalid()) {
        enc = (int) multicore_fifo_pop_blocking();
    }

    // sleep check
    sleep_ctr = (usb_connected || sleep_check(enc)) ? SLEEP_TIME : sleep_ctr - 1;
    gpio_put(sleep, !(sleep_ctr == 0 || sleep_ctr > SLEEP_PULSE));

    // TODO update to actual GUI
    // no press => red, press => green
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            leds(i, j) = keys(i, j) ? ws2812b_color(0, 20, 20) : ws2812b_color(30, 0, 0);
        }
    }

    // display sleep time
    enc_ticks += enc;
    oled.clear();
    oled.set_cursor(0, 0);
    oled_printf("sleep countdown: %ds", sleep_ctr / 1000);
    oled.set_cursor(0, 8);
    oled_printf("encoder ticks: %d", enc_ticks);
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
