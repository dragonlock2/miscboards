#ifndef WS2812B_H
#define WS2812B_H

#include <pico/stdlib.h>

struct ws2812b_color_t {
    uint8_t g;
    uint8_t r;
    uint8_t b;
};

class ws2812b {
public:
    ws2812b(uint rows, uint cols, const uint* led_pins);
    ws2812b_color_t& operator() (uint row, uint col);
    void display(void);

private:
    const uint rows;
    const uint cols;
    const uint* const led_pins;
    ws2812b_color_t pixels[MAX_ROWS][MAX_COLS];
};

#endif // WS2812B_H
