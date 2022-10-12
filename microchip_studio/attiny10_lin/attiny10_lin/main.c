#define F_CPU 8000000L

#include <xc.h>
#include <stdbool.h>
#include <util/delay.h>

int main(void) {
	// prescaler to 1 (8MHz)
	CCP = 0xD8;
	CLKPSR = 0x00;
	
	// PB0 as output
	DDRB |= (1 << DDB0);
	
    while (true) {
		PORTB |= (1 << DDB0);
        _delay_ms(500);
		PORTB &= ~(1 << DDB0);
		_delay_ms(500);
    }
}
