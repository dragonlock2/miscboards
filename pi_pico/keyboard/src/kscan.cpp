#include <string.h>
#include "kscan.h"

kscan::kscan(uint rows, uint cols, const uint* row_pins, const uint* col_pins)
:
    rows(rows), cols(cols), row_pins(row_pins), col_pins(col_pins)
{
    // init all row pins to outputs
    for (uint i = 0; i < rows; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], true);
        gpio_put(row_pins[i], 0);
    }
    
    // init all col pins to inputs, pulldown enabled
    for (uint i = 0; i < cols; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], false);
        gpio_pull_down(col_pins[i]);
    }

    memset(debounce, 0, sizeof(debounce));
    memset(pressed,  0, sizeof(pressed));
}

void kscan::scan(void) {
    for (uint r = 0; r < rows; r++) {
        gpio_put(row_pins[r], 1);
        sleep_us(1); // delay << settle time
        for (uint c = 0; c < cols; c++) {
            // "instant" debounce alg, enforces minimum time between state changes instead
            bool s = gpio_get(col_pins[c]);
            if (debounce[r][c] == 0) {
                if (s != pressed[r][c]) {
                    debounce[r][c] = DEBOUNCE_TIME;
                }
                pressed[r][c] = s;
            } else {
                debounce[r][c]--;
            }
        }
        gpio_put(row_pins[r], 0);
    }
}

bool kscan::operator() (uint row, uint col) {
    return pressed[row][col];
}
