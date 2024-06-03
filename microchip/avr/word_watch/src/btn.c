#include <avr/interrupt.h>
#include <avr/io.h>
#include "btn.h"

/* private data */
static struct {
    bool waked;
} data;

/* private helpers */
ISR(PORTC_PORT_vect) {
    PORTC.INTFLAGS = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
    data.waked = true;
}

/* public functions */
void btn_init(void) {
    PORTC.DIRCLR   = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
    PORTC.PIN0CTRL = PORT_INVEN_bm | PORT_PULLUPEN_bm;
    PORTC.PIN1CTRL = PORT_INVEN_bm | PORT_PULLUPEN_bm;
    PORTC.PIN2CTRL = PORT_INVEN_bm | PORT_PULLUPEN_bm;
    PORTC.PIN3CTRL = PORT_INVEN_bm | PORT_PULLUPEN_bm;
}

void btn_read(btn_t *status) {
    uint8_t val = PORTC.IN;
    status->up     = val & PIN1_bm;
    status->center = val & PIN2_bm;
    status->down   = val & PIN3_bm;
    status->shake  = val & PIN0_bm;
}

bool btn_waked(void) {
    return data.waked;
}

void btn_sleep(void) {
    PORTC.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;
    data.waked = false;
}

void btn_wake(void) {
    PORTC.PIN0CTRL &= ~PORT_ISC_gm;
    PORTC.PIN1CTRL &= ~PORT_ISC_gm;
    PORTC.PIN2CTRL &= ~PORT_ISC_gm;
    PORTC.PIN3CTRL &= ~PORT_ISC_gm;
}
