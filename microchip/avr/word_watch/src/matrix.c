#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "matrix.h"
#include "matrix_gen.h"

/* private data */
static struct {
    bool frame_curr;
    uint8_t frame_idx;
    bool frame[2][MATRIX_LENGTH];
} data;

/* private helpers */
static inline void matrix_update(void) {
    PORTA.DIRCLR = MATRIX_PORTA_CLR;
    PORTB.DIRCLR = MATRIX_PORTB_CLR;
    if (data.frame[data.frame_curr][data.frame_idx]) {
        const matrix_lut_t *m = &MATRIX_LUT[data.frame_idx];
        PORTA.OUTSET = m->porta_outset;
        PORTA.OUTCLR = m->porta_outclr;
        PORTB.OUTSET = m->portb_outset;
        PORTB.OUTCLR = m->portb_outclr;
        PORTA.DIRSET = m->porta_dirset;
        PORTB.DIRSET = m->portb_dirset;
    }

    data.frame_idx++;
    if (data.frame_idx >= MATRIX_LENGTH) {
        data.frame_idx = 0;
    }
}

ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm;
    matrix_update();
}

/* public functions */
void matrix_init(void) {
    TCB0.CTRLA   = TCB_CLKSEL_CLKDIV2_gc;
    TCB0.CTRLB   = TCB_CNTMODE_INT_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CCMP    = F_CPU / 2 / MATRIX_LENGTH / 90 - 1; // 90fps
    TCB0.CTRLA  |= TCB_ENABLE_bm;

    data.frame_curr = false;
    data.frame_idx  = 0;
}

void matrix_set(uint8_t x, uint8_t y) {
    data.frame[!data.frame_curr][y * MATRIX_WIDTH + x] = true;
}

void matrix_display(void) {
    cli();
    data.frame_curr = !data.frame_curr;
    sei();
    memset(data.frame[!data.frame_curr], 0, MATRIX_LENGTH);
}

void matrix_sleep(void) {
    TCB0.CTRLA    &= ~TCB_ENABLE_bm;
    TCB0.INTFLAGS  = TCB_CAPT_bm;

    // floating draws extra current bc LEDs are sensors
    PORTA.DIRCLR = MATRIX_PORTA_CLR;
    PORTB.DIRCLR = MATRIX_PORTB_CLR;
    PORTA.OUTCLR = MATRIX_PORTA_CLR;
    PORTB.OUTCLR = MATRIX_PORTB_CLR;
    PORTA.DIRSET = MATRIX_PORTA_CLR;
    PORTB.DIRSET = MATRIX_PORTB_CLR;

    // don't display old frame on wake
    memset(data.frame[data.frame_curr], 0, MATRIX_LENGTH);
}

void matrix_wake(void) {
    TCB0.CTRLA |= TCB_ENABLE_bm;
}
