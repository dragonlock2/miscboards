#ifndef TICKER_H
#define TICKER_H

#include <stdint.h>

#if defined(ch32v003)
#include <ch32v00x.h>
#elif defined(ch32v20x)
#include <ch32v20x.h>
#elif defined(ch32x035)
#include <ch32x035.h>
#else
#error "define mcu!"
#endif

void ticker_start(uint64_t period, void (*cb)(void));

#endif // TICKER_H
