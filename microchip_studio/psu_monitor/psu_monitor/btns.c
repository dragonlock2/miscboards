#include "mcc_generated_files/mcc.h"
#include "btns.h"

/* private defines */
#define BTNS_SHORT_THRESHOLD (10) // # of loop iterations
#define BTNS_LONG_THRESHOLD  (30)

typedef struct {
    bool pressed;
    uint8_t pressed_counter;
    btns_press_E press_type;
} btns_state_S;

typedef struct {
    btns_state_S buttons[BTNS_CHANNEL_COUNT];
} btns_data_S;
static btns_data_S btns_data;

/* private helpers */
bool btns_read_raw(btns_channel_E chan) {
    switch (chan) {
        case BTNS_CHANNEL_LEFT:   return !BTN0_GetValue(); break;
        case BTNS_CHANNEL_RIGHT:  return !BTN2_GetValue(); break;
        case BTNS_CHANNEL_CENTER: return !BTN1_GetValue(); break;
        default: return false; break;
    }
}

/* public functions */
void btns_init() {
    for (int i = 0; i < BTNS_CHANNEL_COUNT; i++) {
        btns_data.buttons[i].pressed = false;
        btns_data.buttons[i].press_type = BTNS_PRESS_NONE;
    }
}

void btns_loop() {
    for (int i = 0; i < BTNS_CHANNEL_COUNT; i++) {
        if (btns_data.buttons[i].pressed) {
            if (btns_read_raw(i)) {
                btns_data.buttons[i].pressed_counter++;
                if (btns_data.buttons[i].pressed_counter > BTNS_LONG_THRESHOLD) {
                    btns_data.buttons[i].pressed_counter = BTNS_LONG_THRESHOLD;
                    btns_data.buttons[i].press_type = BTNS_PRESS_HOLD;
                }
            } else {
                btns_data.buttons[i].pressed = false;
                if (btns_data.buttons[i].press_type == BTNS_PRESS_NONE) {
                    btns_data.buttons[i].press_type = btns_data.buttons[i].pressed_counter < BTNS_SHORT_THRESHOLD ? BTNS_PRESS_SHORT : BTNS_PRESS_LONG;
                }                
            }
        } else {
            btns_data.buttons[i].press_type = BTNS_PRESS_NONE;
            if (btns_read_raw(i)) {
                btns_data.buttons[i].pressed = true;
                btns_data.buttons[i].pressed_counter = 0;
                // loop length should be enough to debounce
            }
        }
    }
}

btns_press_E btns_read(btns_channel_E chan) {
    return btns_data.buttons[chan].press_type;
}
