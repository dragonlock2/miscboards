#pragma once

#include <cstdbool>
#include <cstdint>

namespace usr {

using int_callback = void (*)(void *arg);

uint8_t id();
void phyint(int_callback cb, void *arg);
void phyrst(bool assert);
void rgb(bool r, bool g, bool b);
bool btn();

};
