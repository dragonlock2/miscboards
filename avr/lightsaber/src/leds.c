#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "leds.h"

/* private defines */
#define LED_PORT (PORTC)
#define LED_MASK (PIN0_bm | PIN1_bm)

/* private helpers */
static inline void leds_write_bit(bool b) {
    cli(); // high pulse needs precise timing
    LED_PORT.OUTSET = LED_MASK;
    _delay_loop_1(b ? 4 : 1);
    LED_PORT.OUTCLR = LED_MASK;
    sei(); // low pulse just needs to be <50us (interruptible)
    if (!b) { asm volatile ("nop"); } // adjust delay for loop overhead
}

static inline void leds_write_byte(uint8_t b) {
    for (int i = 0; i < 8; i++) {
        leds_write_bit(b & 0x80);
        b <<= 1;
    }
}

/* public functions */
void leds_init() {
    LED_PORT.DIRSET = LED_MASK;
    LED_PORT.OUTCLR = LED_MASK;
}

void leds_sleep() {
    LED_PORT.DIRCLR = LED_MASK;
}

void leds_wake() {
    LED_PORT.DIRSET = LED_MASK;
}

void leds_write(uint8_t r, uint8_t g, uint8_t b) {
    leds_write_byte(g);
    leds_write_byte(r);
    leds_write_byte(b);
}
