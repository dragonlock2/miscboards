#include <avr/io.h>

int main(void) {
    // divider = 1 => 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    // TODO stuff
}
