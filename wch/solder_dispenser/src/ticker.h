#pragma once
#include <cstdbool>
#include <cstdint>

void ticker_start(uint32_t freq, volatile bool* sig);
