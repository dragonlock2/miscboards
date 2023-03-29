#include "encoder.h"

// based on https://forum.arduino.cc/t/reading-rotary-encoders-as-a-state-machine/937388
using enum encoder_state;
static const encoder_state ENCODER_STATE_LUT[(uint) COUNT][4] = {
    //                 00,     01,     10,     11
    [(uint) DETENT] = {DETENT, CW1,    CCW1,   DETENT}, // 11
    [(uint) CW1]    = {CW2,    CW1,    DETENT, DETENT}, // 01
    [(uint) CW2]    = {CW2,    CW1,    CW3,    DETENT}, // 00
    [(uint) CW3]    = {CW2,    DETENT, CW3,    DETENT}, // 10
    [(uint) CCW1]   = {CCW2,   DETENT, CCW1,   DETENT}, // 10
    [(uint) CCW2]   = {CCW2,   CCW3,   CCW1,   DETENT}, // 00
    [(uint) CCW3]   = {CCW2,   CCW3,   DETENT, DETENT}, // 01
};

encoder::encoder(uint a, uint b)
:
    state(DETENT), a(a), b(b), ticks(0)
{
    gpio_init(a);
    gpio_set_dir(a, false);
    gpio_pull_up(a);

    gpio_init(b);
    gpio_set_dir(b, false);
    gpio_pull_up(b);
}

void encoder::scan(void) {
    // 00 <> 10 <> 11 <> 01 <> 00
    uint i = (gpio_get(a) << 1) | gpio_get(b);
    encoder_state n = ENCODER_STATE_LUT[(uint) state][i];
    if (state == CW3 && n == DETENT) {
        ticks++;
    } else if (state == CCW3 && n == DETENT) {
        ticks--;
    }
    state = n;
}

encoder::operator int() {
    int t = ticks;
    // ticks = 0;
    return t;
}
