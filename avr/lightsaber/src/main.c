#define F_CPU 3300000L

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    PORTC.DIRSET = PIN0_bm;

    while (1) {
        // TODO call memcpy and pow and printf
        PORTC.OUTTGL = PIN0_bm;
        _delay_ms(500);
    }
}
