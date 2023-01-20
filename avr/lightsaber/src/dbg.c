#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dbg.h"

/* private helpers */
int uart_putc(char var, FILE *stream) {
    // TODO use UART
    PORTC.OUTTGL = PIN0_bm;
    _delay_ms(20);
    return 0;
}

/* public functions */
FILE mystdout = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);

void dbg_init() {
    // TODO setup UART
    PORTC.DIRSET = PIN0_bm;

    stdout = &mystdout;
}
