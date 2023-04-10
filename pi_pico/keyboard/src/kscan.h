#ifndef KSCAN_H
#define KSCAN_H

#include <pico/stdlib.h>
#include "config.h"

class kscan {
public:
    kscan(const kb_config& cfg);
    void scan(void);
    bool operator() (uint row, uint col);

    const uint rows;
    const uint cols;

private:
    const uint* const row_pins;
    const uint* const col_pins;
    uint debounce[MAX_ROWS][MAX_COLS];
    bool pressed[MAX_ROWS][MAX_COLS];
};

#endif // KSCAN_H
