#ifndef CONFIG_H
#define CONFIG_H

#include <pico/stdlib.h>

#define MAX_ROWS (16)
#define MAX_COLS (16)

// board-specific config
#define DEBOUNCE_TIME (5) //ms
static const uint NUM_ROWS = 5;
static const uint NUM_COLS = 9;
static const uint ROW_PINS[NUM_ROWS] = { 0, 2, 4, 6, 8 };
static const uint COL_PINS[NUM_COLS] = { 16, 17, 18, 15, 14, 13, 12, 11, 10 };

#define LED_COL_REVERSE (true)
static const uint LED_PINS[NUM_ROWS] = { 1, 3, 5, 7, 9 };

#endif // CONFIG_H
