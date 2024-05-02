#include <array>
#include "btn.h"

#define LONG_PRESS (500)

struct btn_data {
    bool prev;
    bool pressed;
};

static std::array<btn_data, static_cast<int>(btn::count)> data;

void btn_run(void) {
    for (int i = 0; i < static_cast<int>(btn::count); i++) {
        bool curr = btn_read(static_cast<btn>(i));
        data[i].pressed = curr && !data[i].prev;
        data[i].prev = curr;
    }
}

bool btn_pressed(btn c) {
    return data[static_cast<int>(c)].pressed;
}
