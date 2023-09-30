#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "monitor.h"
#include "power.h"

/* private data */
#define SLEEP_MIN_WIDTH (5) // 50ms

static uint8_t sleep_ctr = 0;

/* private helpers */
static inline bool power_sleep_request(void) {
    return !(PINB & (1 << 1)) || (PINB & (1 << 2)); // pwr_sw or "watchdog"
}

static inline void power_sleep(void) {
    GIMSK |=  (1 << 6); // enable INT0
    sleep_mode();
    GIMSK &= ~(1 << 6); // disable INT0
    while (power_sleep_request()); // wait until release pwr_sw
}

ISR(INT0_vect) {
    // nothing here
}

/* public functions */
void power_init(void) {
    DDRB  |= (1 << 4);
    PORTB |= (1 << 1) | (1 << 4); // enable pwr_en, pull-up on pwr_sw
    // MCUCR &= ~(0x03); // INT0 low-level interrupt
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void power_run10ms(void) {
    if (power_sleep_request()) {
        sleep_ctr++;
        if (sleep_ctr > SLEEP_MIN_WIDTH) {
            sleep_ctr = SLEEP_MIN_WIDTH;
        }
    } else {
        if (sleep_ctr == SLEEP_MIN_WIDTH) { // high pulse > min width => sleep
            PORTB &= ~(1 << 4); // disable pwr_en
            monitor_sleep();
            power_sleep();
            monitor_wake();
            PORTB |=  (1 << 4); // enable pwr_en
        }
        sleep_ctr = 0;
    }
}
