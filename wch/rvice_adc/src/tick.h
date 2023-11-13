#ifndef TICK_H
#define TICK_H

#include <cstdint>

void tick_init(uint64_t freq, void (*cb)(void));

#endif // TICK_H
