#include <array>
#include "btn.h"

static struct {
    std::array<bool, static_cast<int>(btn::count)> prev;
    std::array<bool, static_cast<int>(btn::count)> pressed;
} data;

void btn_run(void) {
    for (int i = 0; i < static_cast<int>(btn::count); i++) {
        bool curr = btn_read(static_cast<btn>(i));
        data.pressed[i] = curr && !data.prev[i];
        data.prev[i] = curr;
    }
}

bool btn_pressed(btn c) {
    return data.pressed[static_cast<int>(c)];
}
