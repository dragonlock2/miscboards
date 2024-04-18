#include <array>
#include "btn.h"

#define LONG_PRESS (500)

struct btn_data {
    bool prev;
    bool pressed;
    int press_len;
};

static std::array<btn_data, static_cast<int>(btn::count)> data;

void btn_run(void) {
    for (int i = 0; i < static_cast<int>(btn::count); i++) {
        bool curr = btn_read(static_cast<btn>(i));
        data[i].pressed = curr && !data[i].prev;
        data[i].prev = curr;
        if (curr) {
            data[i].press_len++;
            if (data[i].press_len >= LONG_PRESS) {
                data[i].press_len = LONG_PRESS;
            }
        } else {
            data[i].press_len = 0;
        }
    }
}

bool btn_pressed(btn c) {
    return data[static_cast<int>(c)].pressed;
}

bool btn_long_pressed(btn c) {
    return data[static_cast<int>(c)].press_len >= LONG_PRESS;
}
