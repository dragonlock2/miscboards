#include <cstdio>
#include <pico/stdlib.h>
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
            leds(i, j) = keys(i, j) ? ws2812b_color(0, 40, 0) : ws2812b_color(40, 0, 0);
        }
    }

    // display sleep time
    oled.clear();
    oled.set_cursor(0, 0);
    oled_printf("sleep countdown: %ds", sleep_ctr / 1000);

    // sleep counter
    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                sleep_ctr = SLEEP_TIME; // TODO also wake if encoder touched
            }
        }
    }
    sleep_ctr--;
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
