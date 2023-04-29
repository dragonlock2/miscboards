#include <stdio.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include "dbg.h"

#if (F_CPU == 20000000L)
#define BIT_TIME (55) // 115200bps
#endif

#define TX_PORT (PORTC)
#define TX_PIN  (PIN3_bm)

/* private helpers */
static int dbg_putc(char c, FILE *f) {
    // start bit
    TX_PORT.OUTCLR = TX_PIN;
    _delay_loop_1(BIT_TIME);
    // bits
    for (int i = 0; i < 8; i++) {
        if (c & 0x01) {
            TX_PORT.OUTSET = TX_PIN;
        } else {
            TX_PORT.OUTCLR = TX_PIN;
        }
        c >>= 1;
        _delay_loop_1(BIT_TIME);
    }
    // stop bit
    TX_PORT.OUTSET = TX_PIN;
    _delay_loop_1(BIT_TIME);
    return 0;
}

/* public functions */
void dbg_init() {
    // TX (PC3)
    TX_PORT.DIRSET = TX_PIN;
    TX_PORT.OUTSET = TX_PIN;

    static FILE uart_stdout = FDEV_SETUP_STREAM(dbg_putc, NULL, _FDEV_SETUP_WRITE);
    stdout = &uart_stdout;
}
