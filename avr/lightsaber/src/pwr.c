#include <avr/io.h>
#include "pwr.h"

/* private defines */
#define PWR_PORT (PORTB)
#define PWR_PIN  (PIN3_bm)

/* public functions */
void pwr_init() {
    PWR_PORT.DIRSET = PWR_PIN;
    PWR_PORT.OUTSET = PWR_PIN;
}
