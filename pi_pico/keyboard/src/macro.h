#ifndef MACRO_H
#define MACRO_H

#include <tusb.h>
#include "kscan.h"
#include "config.h"

class macro {
public:
    macro(kscan& keys, const kb_config& cfg, const kb_map& map);

    void get_report(int& ticks, bool consumer_txd, hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer);

private:
    const kb_config& cfg;
    const kb_map&    map;
    kscan&           keys;

    uint    tick_ctr;
    bool    ticked;
    uint8_t tick_key;

    bool     prev_keys[MAX_ROWS][MAX_COLS];
    uint     consumer_ctr;
    uint16_t consumer_key;
};

#endif // MACRO_H
