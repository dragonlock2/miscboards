#pragma once

#include <cstdbool>
#include <cstdint>

namespace usr {

typedef void (*int_callback)(void *arg);

uint8_t id(void);
void phyint(int_callback cb, void *arg);
void phyrst(bool assert);
void rgb(bool r, bool g, bool b);
bool btn(void);

};
