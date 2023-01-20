#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dbg.h"

int main(void) {
    // set to 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    // init
    dbg_init();

    // loop
    while (1) {
        puts("hello world!\r\n");
        _delay_ms(500);
    }
}
