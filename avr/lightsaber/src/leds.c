#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "leds.h"

/* private defines */
#define TOP_MASK (PIN1_bm)
#define BOT_MASK (PIN2_bm)

/* private helpers */
static inline void leds_write_bit(bool top, bool b) {
    const uint8_t mask = top ? TOP_MASK : BOT_MASK;
    cli(); // high pulse needs precise timing
    PORTC.OUTSET = mask;
    _delay_loop_1(b ? 4 : 1);
    PORTC.OUTCLR = mask;
    sei(); // low pulse just needs to be <50us (interruptible)
    if (!b) { asm volatile ("nop"); } // adjust delay for loop overhead
}

static inline void leds_write_byte(bool top, uint8_t b) {
    for (int i = 0; i < 8; i++) {
        leds_write_bit(top, b & 0x80);
        b <<= 1;
    }
}

/* public functions */
void leds_init() {
    /* top (PC1), bot (PC2) */
    PORTC.DIRSET = TOP_MASK | BOT_MASK;
    PORTC.OUTCLR = TOP_MASK | BOT_MASK;
}

void leds_write(bool top, uint8_t r, uint8_t g, uint8_t b) {
    leds_write_byte(top, g);
    leds_write_byte(top, r);
    leds_write_byte(top, b);
}
