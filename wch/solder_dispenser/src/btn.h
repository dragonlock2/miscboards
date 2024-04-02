#pragma once
#include <cstdbool>

enum class btn {
    up,
    down,
    dispense,
};

bool btn_read(btn c);
void btn_sleep(void);
void btn_wake(void);
