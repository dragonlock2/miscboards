#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "btn.h"
#include "clock.h"
#include "dbg.h"
#include "matrix.h"
#include "pwr.h"

/**
 * ITMISSHALF
 * QUARTERTEN
 * TWENTYFIVE
 * PASTOPFOUR
 * TWONESEVEN
 * THREELEVEN
 * FIVETENINE
 * EIGHTWELVE
 * SIXYAMPM--
 */
static inline void render(void) {
    int8_t h, m, s;
    clock_now(&h, &m, &s);

    // compute helpers
    bool am;
    if (h < 12) {
        am = true;
    } else {
        h -= 12;
        am = false;
    }

    s = m % 5;
    m -= s;

    bool past;
    if (m <= 30) {
        past = true;
    } else {
        past = false;
        if (h == 11) {
            am = !am;
        }
        h++;
        m = 60 - m;
    }

    // display words
    matrix_set(0, 0);
    matrix_set(1, 0);
    matrix_set(3, 0);
    matrix_set(4, 0);

    switch (m) {
        case 5:
            matrix_set(6, 2);
            matrix_set(7, 2);
            matrix_set(8, 2);
            matrix_set(9, 2);
            break;
        case 10:
            matrix_set(7, 1);
            matrix_set(8, 1);
            matrix_set(9, 1);
            break;
        case 15:
            matrix_set(7, 0);
            matrix_set(0, 1);
            matrix_set(1, 1);
            matrix_set(2, 1);
            matrix_set(3, 1);
            matrix_set(4, 1);
            matrix_set(5, 1);
            matrix_set(6, 1);
            break;
        case 20:
            matrix_set(0, 2);
            matrix_set(1, 2);
            matrix_set(2, 2);
            matrix_set(3, 2);
            matrix_set(4, 2);
            matrix_set(5, 2);
            break;
        case 25:
            matrix_set(0, 2);
            matrix_set(1, 2);
            matrix_set(2, 2);
            matrix_set(3, 2);
            matrix_set(4, 2);
            matrix_set(5, 2);
            matrix_set(6, 2);
            matrix_set(7, 2);
            matrix_set(8, 2);
            matrix_set(9, 2);
            break;
        case 30:
            matrix_set(6, 0);
            matrix_set(7, 0);
            matrix_set(8, 0);
            matrix_set(9, 0);
            break;
    }

    if (m != 0) {
        if (past) {
            matrix_set(0, 3);
            matrix_set(1, 3);
            matrix_set(2, 3);
            matrix_set(3, 3);
        } else {
            matrix_set(3, 3);
            matrix_set(4, 3);
        }
    }

    switch (h) {
        case 0:
        case 12:
            matrix_set(4, 7);
            matrix_set(5, 7);
            matrix_set(6, 7);
            matrix_set(7, 7);
            matrix_set(8, 7);
            matrix_set(9, 7);
            break;
        case 1:
            matrix_set(2, 4);
            matrix_set(3, 4);
            matrix_set(4, 4);
            break;
        case 2:
            matrix_set(0, 4);
            matrix_set(1, 4);
            matrix_set(2, 4);
            break;
        case 3:
            matrix_set(0, 5);
            matrix_set(1, 5);
            matrix_set(2, 5);
            matrix_set(3, 5);
            matrix_set(4, 5);
            break;
        case 4:
            matrix_set(6, 3);
            matrix_set(7, 3);
            matrix_set(8, 3);
            matrix_set(9, 3);
            break;
        case 5:
            matrix_set(0, 6);
            matrix_set(1, 6);
            matrix_set(2, 6);
            matrix_set(3, 6);
            break;
        case 6:
            matrix_set(0, 8);
            matrix_set(1, 8);
            matrix_set(2, 8);
            break;
        case 7:
            matrix_set(5, 4);
            matrix_set(6, 4);
            matrix_set(7, 4);
            matrix_set(8, 4);
            matrix_set(9, 4);
            break;
        case 8:
            matrix_set(0, 7);
            matrix_set(1, 7);
            matrix_set(2, 7);
            matrix_set(3, 7);
            matrix_set(4, 7);
            break;
        case 9:
            matrix_set(6, 6);
            matrix_set(7, 6);
            matrix_set(8, 6);
            matrix_set(9, 6);
            break;
        case 10:
            matrix_set(4, 6);
            matrix_set(5, 6);
            matrix_set(6, 6);
            break;
        case 11:
            matrix_set(4, 5);
            matrix_set(5, 5);
            matrix_set(6, 5);
            matrix_set(7, 5);
            matrix_set(8, 5);
            matrix_set(9, 5);
            break;
    }

    if (am) {
        matrix_set(4, 8);
        matrix_set(5, 8);
    } else {
        matrix_set(6, 8);
        matrix_set(7, 8);
    }

    switch (s) { // only have 2 leds for 5 states :P
        case 0:
            break;
        case 1:
            matrix_set(9, 8);
            break;
        case 2:
        case 3:
            matrix_set(8, 8);
            break;
        case 4:
            matrix_set(8, 8);
            matrix_set(9, 8);
            break;
    }
}

static inline void render_lowbatt(void) {
    matrix_set(0, 0);
    matrix_set(2, 0);
    matrix_set(3, 0);
    matrix_set(4, 0);
    matrix_set(5, 0);
    matrix_set(1, 1);
}

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

    bool lowbatt = false;
    while (1) {
        pwr_sleep();

        bool wake = btn_waked();
        bool show_time = false;
        uint16_t last_interact = clock_ticks();
        bool up_prev = false, down_prev = false;
        bool up_hold = false, down_hold = false;
        uint16_t up_last = 0, down_last = 0;
        while (wake) {
            uint16_t ticks = clock_ticks();

            // process button
            btn_t btn;
            btn_read(&btn);
            if (btn.up || btn.center || btn.down || btn.shake) {
                last_interact = ticks;
            }

            bool up_rise   = btn.up   && !up_prev;
            bool down_rise = btn.down && !down_prev;
            if (up_rise)   { up_last   = ticks; }
            if (down_rise) { down_last = ticks; }
            up_prev   = btn.up;
            down_prev = btn.down;
            up_hold   = btn.up   && (up_hold   || ((ticks - up_last)   >= 500));
            down_hold = btn.down && (down_hold || ((ticks - down_last) >= 500));

            // ui
            bool time_change = false;
            int8_t h, m, s;
            clock_now(&h, &m, &s);
            if (up_rise || up_hold) {
                time_change = true;
                m++;
            } else if (down_rise || down_hold) {
                time_change = true;
                m--;
            }
            if (time_change) {
                show_time = true;
                clock_set(h, m, 0);
                _delay_ms(10);

                // int8_t h, m, s;
                // clock_now(&h, &m, &s);
                // printf("%d %d %d\r\n", h, m, s);
            }

            // low battery indicator
            uint16_t vbat = pwr_vbat();
            if (lowbatt) {
                lowbatt = vbat < 3700;
            } else {
                lowbatt = vbat < 3400;
            }

            // render
            if (!show_time && lowbatt) {
                render_lowbatt();
                if ((ticks - last_interact) >= 500) {
                    show_time = true;
                }
            } else {
                render();
            }
            matrix_display();

            // sleep timeout
            if ((ticks - last_interact) >= 2000) { // wake for ~2s
                wake = false;
            }
        }
    }
}
