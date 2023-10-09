#ifndef TICKER_H
#define TICKER_H

#include <stdint.h>

void ticker_start(uint64_t period, void (*cb)(void));

#endif // TICKER_H
