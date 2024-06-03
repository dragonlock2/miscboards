#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "btn.h"
#include "clock.h"
#include "dbg.h"
#include "matrix.h"
#include "pwr.h"

int main(void) {
    // constructor priorities not supported :(
    cli();
    dbg_init();
    btn_init();
    clock_init();
    matrix_init();
    pwr_init();
    sei();
    printf("booted!\r\n");

    // TODO sleep
    // TODO app
    while (1);
}
