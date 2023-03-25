#include "config.h"
#include "kscan.h"
#include "ws2812b.pio.h"
#include "ws2812b.h"

ws2812b::ws2812b(uint rows, uint cols, const uint* led_pins)
:
    rows(rows), cols(cols), led_pins(led_pins)
{
    // TODO write own PIO to do all in parallel and use DMA on one PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PINS[0], 800000, false);
    pio_sm_put_blocking(pio, sm, 0x000A00 << 8u);
    pio_sm_put_blocking(pio, sm, 0x000A00 << 8u);
}

ws2812b_color_t& ws2812b::operator() (uint row, uint col) {
#if LED_COL_REVERSE
    return pixels[row][cols - 1 - col];
#else
    return pixels[row][col];
#endif
}

void ws2812b::display(void) {

}
