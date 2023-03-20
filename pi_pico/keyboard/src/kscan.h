#ifndef KSCAN_H
#define KSCAN_H

#include <pico/stdlib.h>

#define KSCAN_ROWS (5)
#define KSCAN_COLS (9)

void kscan_init(void);
void kscan_run1ms(void);
bool kscan_read(uint row, uint col);

#endif // KSCAN_H
