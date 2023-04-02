#ifndef MACRO_H
#define MACRO_H

#include <tusb.h>
#include "kscan.h"
#include "config.h"

class macro {
public:
    macro(kscan& keys, const uint8_t keymap[MAX_ROWS][MAX_COLS], const uint8_t keymodmap[MAX_ROWS][MAX_COLS], const uint8_t encmap[2],
        const uint16_t consumer_keymap[MAX_ROWS][MAX_COLS], const uint16_t consumer_encmap[2]);

    void get_report(int& ticks, hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer);

private:
    uint8_t  keymap[MAX_ROWS][MAX_COLS];
    uint8_t  keymodmap[MAX_ROWS][MAX_COLS];
    uint8_t  encmap[2];
    uint16_t consumer_keymap[MAX_ROWS][MAX_COLS];
    uint16_t consumer_encmap[2];

    kscan& keys;

    uint     tick_ctr;
    bool     ticked;
    uint8_t  tick_key;

    bool     prev_keys[MAX_ROWS][MAX_COLS];
    uint     consumer_ctr;
    uint16_t consumer_key;
};

#endif // MACRO_H
