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

    // TODO app
        // time display
        // time change
        // low battery indicator
        // sleep timeout
    while (1) {
        pwr_sleep();
        if (btn_waked()) {
            uint8_t h, m, s;
            clock_now(&h, &m, &s);
            for (uint8_t x = 0; x < 10; x++) {
                for (uint8_t y = 0; y < 9; y++) {
                    if ((y * 10 + x) <= s) {
                        matrix_set(x, y);
                    }
                }
            }
            matrix_display();
            printf("%d %d %d\r\n", h, m, s);
            _delay_ms(1000);
        }
    }
}
