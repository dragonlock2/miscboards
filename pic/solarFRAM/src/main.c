#include <xc.h>
#include "config.h"

int main(void) {
    OSCCONbits.IRCF = 0b1010; // select FOSC = 500kHz

    TRISA2 = 0;
    while (1) {
        RA2 = 1;
        __delay_ms(250);
        RA2 = 0;
        __delay_ms(250);
    }
}
