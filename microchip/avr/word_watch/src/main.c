#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "dbg.h"

int main(void) {
    // constructor priorities not supported :(
    cli();
    dbg_init();
    sei();
    printf("booted!\r\n");

    // TODO matrix
    // TODO vbat
    // TODO clock
    // TODO buttons/shake
    // TODO sleep
}
