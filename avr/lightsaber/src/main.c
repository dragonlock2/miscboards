#define F_CPU 20000000L

#include <avr/io.h>
#include <util/delay.h>
// #include <stdio.h>

int main(void) {
    // set to 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    PORTC.DIRSET = PIN0_bm;
    while (1) {
        // puts("hello world!\r\n");

        PORTC.OUTTGL = PIN0_bm;
        _delay_ms(500);
    }
}
