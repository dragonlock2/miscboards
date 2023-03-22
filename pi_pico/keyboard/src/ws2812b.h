#ifndef WS2812B_H
#define WS2812B_H

#include <pico/stdlib.h>

typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} ws2812b_color_S;

void ws2812b_init(void);
void ws2812b_set(uint row, uint col, const ws2812b_color_S *color);
void ws2812b_display(void);

#endif // WS2812B_H
