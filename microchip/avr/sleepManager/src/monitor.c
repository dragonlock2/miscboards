#include <stdbool.h>
#include <avr/io.h>
#include "monitor.h"

/* private data */
#define MV_TO_ADC(x) ((uint16_t)(x * 1.0 / (1.0 + 10.0) / 1100.0 * 1024.0))
#define FLASH_PERIOD (25) // 250ms

struct {
    bool low;
    uint16_t counter;
} monitor_data;

/* private helpers */
static inline uint16_t monitor_read_vbat(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    uint16_t ret = ADCL; // ADCL **MUST** be read first
    ret |= (ADCH << 8);
    return ret;
}

/* public functions */
void monitor_init(void) {
    DDRB  |= (1 << 0);
    PORTB |= (1 << 0); // turn off pwr_led

    ADMUX  = 0x43; // 1.1Vref, select AIN3 (PB3)
    ADCSRA = 0x83; // /8 prescaler => 150kHz
    monitor_data.low = false;
}

void monitor_run10ms(void) {
    uint16_t vbat = monitor_read_vbat();
    if (vbat < MV_TO_ADC(3500)) {
        monitor_data.low = true;
    } else if (vbat > MV_TO_ADC(4000)) {
        monitor_data.low = false;
    }

    if (monitor_data.low) {
        monitor_data.counter++;
        if (monitor_data.counter == FLASH_PERIOD) {
            monitor_data.counter = 0;
            PORTB ^= (1 << 0);
        }
    }
}

void monitor_sleep(void) {
    PORTB  |= (1 << 0); // turn off pwr_led
    ADCSRA &= ~(1 << ADEN); // disable ADC
}

void monitor_wake(void) {
    ADCSRA |= (1 << ADEN); // enable ADC
}
