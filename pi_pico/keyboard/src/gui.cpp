#include <cstdio>
#include <pico/stdlib.h>
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

void GUI::process(void) {
    // no press => red, press => green
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            leds(i, j) = keys(i, j) ? ws2812b_color(0, 20, 20) : ws2812b_color(30, 0, 0);
        }
    }

    // display sleep time
    oled.clear();
    oled.set_cursor(0, 0);
    oled_printf("sleep countdown: %ds", sleep_ctr / 1000);

    // sleep check
    sleep_ctr = sleep_check() ? SLEEP_TIME : sleep_ctr - 1;
    gpio_put(sleep, !(sleep_ctr == 0 || sleep_ctr > SLEEP_PULSE));
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

bool GUI::sleep_check(void) {
    // TODO keep awake if encoder changed
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                return true;
            }
        }
    }
    if (tud_mounted() && !tud_suspended()) {
        return true;
    }
    return false;
}
