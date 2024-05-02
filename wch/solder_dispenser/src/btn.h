#pragma once
#include <cstdbool>

enum class btn {
    up,
    down,
    dispense,
    count,
};

void btn_run(void);
bool btn_pressed(btn c);

bool btn_read(btn c);
void btn_sleep(void);
void btn_wake(void);
