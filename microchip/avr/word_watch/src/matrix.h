#pragma once

#include <stdint.h>

void matrix_init(void);
void matrix_set(uint8_t x, uint8_t y);
void matrix_display(void);
void matrix_sleep(void);
void matrix_wake(void);
