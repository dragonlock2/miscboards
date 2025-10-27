#pragma once

#include <cstdbool>
#include <cstdint>

namespace usr {

using int_callback_t = void (*)(void *arg);

uint8_t id();
void phyint(int_callback_t cb, void *arg);
void phyrst(bool assert);
void rgb(bool r, bool g, bool b);
bool btn();

};
