#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

/* timer */
static volatile bool tick = false;

static void tick_init() {
    
}

int main(void) {
    tick_init();

    DDRB |= (1 << 0) | (1 << 4);
    PORTB |= 1 << 4;

    while (1) {
        

        PORTB |= 1 << 0;
        _delay_ms(500);
        PORTB &= ~(1 << 0);
        _delay_ms(500);

        // 10ms loop
            // check ADC if < 3.5V and go into flashing state until 4V (unless turned off)
            // if switch pressed => go sleep (pull ADC to GND, turn off pwr) => wakeup in same place
            // if sleep request held high then low for 50-100ms => turn off too ^
    }
}
