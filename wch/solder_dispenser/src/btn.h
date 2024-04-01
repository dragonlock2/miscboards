#pragma once
#include <cstdbool>

enum class btn {
    up,
    down,
    dispense,
};

bool btn_read(btn c);
