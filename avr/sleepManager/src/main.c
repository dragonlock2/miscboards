#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "monitor.h"
#include "power.h"

static volatile bool tick = false;

static inline void tick_init(void) {
    TCCR0A = 0x02; // WGM0[1:0]=0b01 (CTC mode)
    TCCR0B = 0x04; // WGM0[2]=0b0, CS0[2:0]=0b100 (/256 prescaler)
    OCR0A  = 46;   // 1200000 / 256 / (46 + 1) = 99.73Hz
    TIMSK0 = 0x04; // enable compare match A output
}

ISR(TIM0_COMPA_vect) {
    tick = true;
}

int main(void) {
    cli();
    monitor_init();
    power_init();
    tick_init();
    sei();

    while (1) {
        while (!tick);
        monitor_run10ms();
        power_run10ms();
        tick = false;
    }
}
