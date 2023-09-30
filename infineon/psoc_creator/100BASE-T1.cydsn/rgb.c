#include <stdbool.h>
#include <project.h>
#include "rgb.h"

/* private helpers */
void rgb_raw(bool r, bool g, bool b) {
    LED_R_Write(!r);
    LED_G_Write(!g);
    LED_B_Write(!b);
}

/* public functions */
void rgb_init() {
    // nothing to see here
}

void rgb(rgb_color_E color) {
    switch (color) {
        case RGB_NONE:  rgb_raw(0, 0, 0); break;
        case RGB_RED:   rgb_raw(1, 0, 0); break;
        case RGB_GREEN: rgb_raw(0, 1, 0); break;
        case RGB_BLUE:  rgb_raw(0, 0, 1); break;
        default:        rgb_raw(0, 0, 0); break;
    }
}
