#include <string.h>
#include "kscan.h"

/* private data */
#define DEBOUNCE_TIME (5) //ms

static const uint ROW_PINS[KSCAN_ROWS] = { 0, 2, 4, 6, 8 };
static const uint COL_PINS[KSCAN_COLS] = { 16, 17, 18, 15, 14, 13, 12, 11, 10 };

struct {
    uint debounce[KSCAN_ROWS][KSCAN_COLS];
    bool pressed[KSCAN_ROWS][KSCAN_COLS];
} kscan_data;

/* public functions */
void kscan_init(void) {
    // init all row pins to outputs
    for (uint i = 0; i < KSCAN_ROWS; i++) {
        gpio_init(ROW_PINS[i]);
        gpio_set_dir(ROW_PINS[i], true);
        gpio_put(ROW_PINS[i], 0);
    }
    
    // init all col pins to inputs, pulldown enabled
    for (uint i = 0; i < KSCAN_COLS; i++) {
        gpio_init(COL_PINS[i]);
        gpio_set_dir(COL_PINS[i], false);
        gpio_pull_down(COL_PINS[i]);
    }

    memset(kscan_data.debounce, 0, sizeof(kscan_data.debounce));
    memset(kscan_data.pressed,  0, sizeof(kscan_data.pressed));
}

void kscan_run1ms(void) {
    for (uint r = 0; r < KSCAN_ROWS; r++) {
        gpio_put(ROW_PINS[r], 1);
        sleep_us(1); // delay << settle time
        for (uint c = 0; c < KSCAN_COLS; c++) {
            // "instant" debounce alg, enforces minimum time between state changes instead
            bool s = gpio_get(COL_PINS[c]);
            if (kscan_data.debounce[r][c] == 0) {
                if (s != kscan_data.pressed[r][c]) {
                    kscan_data.debounce[r][c] = DEBOUNCE_TIME;
                }
                kscan_data.pressed[r][c] = s;
            } else {
                kscan_data.debounce[r][c]--;
            }
        }
        gpio_put(ROW_PINS[r], 0);
    }
}

bool kscan_read(uint row, uint col) {
    return kscan_data.pressed[row][col];
}
