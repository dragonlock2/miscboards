#ifndef WS2812B_H
#define WS2812B_H

#include <pico/stdlib.h>
#include <pico/sem.h>
#include <hardware/pio.h>
#include <hardware/dma.h>

struct __packed ws2812b_color {
    uint32_t pad : 8;
    uint32_t b   : 8;
    uint32_t r   : 8;
    uint32_t g   : 8; // matches WS2812B protocol

    ws2812b_color(void) : b(0), r(0), g(0) {}
    ws2812b_color(uint8_t r, uint8_t g, uint8_t b) : b(b), r(r), g(g) {}
};

class ws2812b {
public:
    ws2812b(uint rows, uint cols, const uint* led_pins);
    ws2812b_color& operator() (uint row, uint col);
    void display(void);

private:
    const uint rows;
    const uint cols;
    const uint* const led_pins;
    ws2812b_color pixels[MAX_ROWS][MAX_COLS]; // TODO double buffer if needed
    PIO pios[MAX_ROWS];
    uint sms[MAX_ROWS];
    uint dmas[MAX_ROWS];
    dma_channel_config dma_cfgs[MAX_ROWS];
    struct semaphore done[MAX_ROWS];
};

#endif // WS2812B_H
