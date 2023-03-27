#include "config.h"
#include "kscan.h"
#include "pio_alloc.h"
#include "ws2812b.pio.h"
#include "ws2812b.h"

#include <cstdio>

ws2812b::ws2812b(uint rows, uint cols, const uint* led_pins)
:
    rows(rows), cols(cols), led_pins(led_pins)
{
    PIO pio = NULL;
    uint offset = 0;
    for (uint i = 0; i < NUM_ROWS; i++) {
        pio_alloc(pios[i], sms[i]);
        if (pios[i] != pio) {
            pio = pios[i];
            offset = pio_add_program(pio, &ws2812b_program); // TODO write own PIO
        }
        ws2812b_program_init(pios[i], sms[i], offset, LED_PINS[i]);
    }
}

ws2812b_color& ws2812b::operator() (uint row, uint col) {
#if LED_COL_REVERSE
    return pixels[row][cols - 1 - col];
#else
    return pixels[row][col];
#endif
}

void ws2812b::display(void) {
    // TODO DMA instead
    for (uint c = 0; c < NUM_COLS; c++) {
        for (uint r = 0; r < NUM_ROWS; r++) {
            uint32_t *color = reinterpret_cast<uint32_t*>(&pixels[r][c]);
            pio_sm_put_blocking(pios[r], sms[r], *color);
        }
    }
}
