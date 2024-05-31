#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "dbg.h"
#include "matrix.h"

int main(void) {
    // constructor priorities not supported :(
    cli();
    dbg_init();
    matrix_init();
    sei();
    printf("booted!\r\n");

    // TODO vbat
    // TODO clock
    // TODO buttons/shake
    // TODO sleep
    // TODO app
    uint8_t x = 0, y = 0;
    while (1) {
        matrix_set(x, y);
        matrix_display();
        x++;
        if (x >= 10) {
            x = 0;
            y++;
            if (y >= 9) {
                y = 0;
            }
        }
        _delay_ms(100);
    }
}
