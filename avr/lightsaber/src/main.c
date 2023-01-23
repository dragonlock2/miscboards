#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dbg.h"

void clk_init() {
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00; // divider = 1 => 20MHz
}

int main(void) {
    // init
    clk_init();
    dbg_init();

    int8_t i = 0;
    PORTC.DIRSET = PIN0_bm;

    // loop
    while (1) {
        printf("hello world! %d\r\n", i++);
        PORTC.OUTTGL = PIN0_bm;
        _delay_ms(100);
    }
}
