#include "macro.h"

// report based system should be universal
#define PRESS_COUNTS    (2) // reports
#define RELEASE_COUNTS  (2) // reports
#define CONSUMER_COUNTS (2) // reports

macro::macro(kscan& keys, const kb_config& cfg, const kb_map& map)
:
    cfg(cfg), map(map), keys(keys), tick_ctr(0), ticked(false), prev_keys({0}), consumer_ctr(0)
{}

void macro::get_report(int& ticks, bool consumer_txd, hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer) {
    uint16_t ckey = 0x0000;
    uint kb_idx = 0;

    uint layer = map.default_layer;
    for (uint i = 0; i < NUM_LAYERS; i++) {
        if ((map.layer_keys[i].row < cfg.num_rows && map.layer_keys[i].col < cfg.num_cols) &&
                keys(map.layer_keys[i].row, map.layer_keys[i].col)) {
            layer = i;
            break;
        }
    }

    if (consumer_txd) {
        if (tick_ctr) { tick_ctr--; }
        if (consumer_ctr) { consumer_ctr--; }
    }

    // encoder overrides everything
    if (tick_ctr == 0 && !ticked && ticks) {
        tick_ctr = PRESS_COUNTS;
        ticked   = true;
        tick_key = map.keyboard.encmap[layer][ticks < 0 ? 0 : 1];
        ckey     = map.consumer.encmap[layer][ticks < 0 ? 0 : 1];
        ticks += ticks < 0 ? 1 : -1;
    } else if (tick_ctr == 0 && ticked) {
        tick_ctr = RELEASE_COUNTS;
        ticked   = false;
    }
    if (ticked && tick_key != HID_KEY_NONE) {
        kb.keycode[kb_idx++] = tick_key;
    }

    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                kb.modifier |= map.keyboard.modmap[layer][i][j];
                if (map.keyboard.map[layer][i][j] != HID_KEY_NONE && kb_idx < sizeof(kb.keycode)) {
                    kb.keycode[kb_idx++] = map.keyboard.map[layer][i][j];
                }
                if (!prev_keys[i][j] && map.consumer.map[layer][i][j] != 0x0000 && ckey == 0x0000) { // rising edge is request
                    ckey = map.consumer.map[layer][i][j];
                }
            }
            prev_keys[i][j] = keys(i, j);
        }
    }

    // Windows doesn't like a held media key...
    if (consumer_ctr) {
        consumer = consumer_key;
    } else if (ckey != 0x0000) {
        consumer_ctr = CONSUMER_COUNTS;
        consumer_key = ckey;
        consumer     = ckey;
    }
}

