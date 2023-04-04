#include "macro.h"

// iOS uses 15ms for BLE
// Windows is 2 volume counts per 5ms
#define PRESS_TIME   (5) // compromise, will miss ticks in iOS
#define RELEASE_TIME (5)

macro::macro(kscan& keys, const uint8_t keymap[MAX_ROWS][MAX_COLS], const uint8_t keymodmap[MAX_ROWS][MAX_COLS], const uint8_t encmap[2],
    const uint16_t consumer_keymap[MAX_ROWS][MAX_COLS], const uint16_t consumer_encmap[2])
:
    keys(keys), tick_ctr(0), ticked(false), prev_keys({0}), consumer_ctr(0)
{
    memcpy(this->keymap,          keymap,          sizeof(this->keymap));
    memcpy(this->keymodmap,       keymodmap,       sizeof(this->keymodmap));
    memcpy(this->encmap,          encmap,          sizeof(this->encmap));
    memcpy(this->consumer_keymap, consumer_keymap, sizeof(this->consumer_keymap));
    memcpy(this->consumer_encmap, consumer_encmap, sizeof(this->consumer_encmap));
}

void macro::get_report(int& ticks, hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer) {
    uint16_t ckey = 0x0000;
    uint kb_idx = 0;

    // encoder overrides everything
    if (tick_ctr == 0 && !ticked && ticks) {
        tick_ctr = PRESS_TIME;
        ticked = true;
        tick_key = encmap[ticks < 0 ? 0 : 1];
        ckey = consumer_encmap[ticks < 0 ? 0 : 1];
        ticks += ticks < 0 ? 1 : -1;
        
    } else if (tick_ctr == 0 && ticked) {
        tick_ctr = RELEASE_TIME;
        ticked = false;
    } else if (tick_ctr) {
        tick_ctr--;
    }
    if (ticked && tick_key != HID_KEY_NONE) {
        kb.keycode[kb_idx++] = tick_key;
    }

    for (uint i = 0; i < keys.rows; i++) {
        for (uint j = 0; j < keys.cols; j++) {
            if (keys(i, j)) {
                kb.modifier |= keymodmap[i][j];
                if (keymap[i][j] != HID_KEY_NONE && kb_idx < sizeof(kb.keycode)) {
                    kb.keycode[kb_idx++] = keymap[i][j];
                }
                if (!prev_keys[i][j] && consumer_keymap[i][j] != 0x0000 && ckey == 0x0000) { // rising edge is request
                    ckey = consumer_keymap[i][j];
                }
            }
            prev_keys[i][j] = keys(i, j);
        }
    }

    // Windows doesn't like a held media key...
    if (consumer_ctr) {
        consumer_ctr--;
        consumer = consumer_key;
    } else if (ckey != 0x0000) {
        consumer_ctr = PRESS_TIME;
        consumer_key = ckey;
        consumer = ckey;
    }
}

