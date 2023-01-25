#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dbg.h"
#include "leds.h"

void clk_init() {
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00; // divider = 1 => 20MHz
}

int main(void) {
    // init
    clk_init();
    dbg_init();
    leds_init();

    // loop
    uint8_t c = 0;
    while (1) {
        leds_write(true, c, 0, 0);
        leds_write(true, 0, c, 0);
        leds_write(true, 0, 0, c);
        c++;
        _delay_ms(2);
    }
}
