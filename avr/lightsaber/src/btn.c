#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "btn.h"

/* private defines */
#define BTN_PORT (PORTA)
#define BTN_PIN  (PIN5_bm)
#define BTN_CTRL (PORTA.PIN5CTRL)
#define BTN_VECT PORTA_PORT_vect

#define BTN_HOLD (150) // ~3s @ 50Hz

/* private data */
static struct {
    bool     prev;
    bool     short_press;
    uint16_t hold_ctr;
} btn_data;

/* private helpers */
ISR(BTN_VECT) {
    BTN_CTRL &= ~PORT_ISC_gm;
}

/* public functions */
void btn_init() {
    BTN_PORT.DIRCLR = BTN_PIN;
    BTN_CTRL = PORT_PULLUPEN_bm;
}

void btn_sleep() {
    BTN_CTRL |= PORT_ISC_LEVEL_gc;
}

void btn_run() {
    bool curr = !(BTN_PORT.IN & BTN_PIN);
    btn_data.short_press = btn_data.prev && !curr && (btn_data.hold_ctr < BTN_HOLD);
    btn_data.hold_ctr = curr ? btn_data.hold_ctr + 1 : 0;
    btn_data.prev = curr;
}

bool btn_pressed() {
    return btn_data.prev;
}

bool btn_hold() {
    return btn_data.hold_ctr >= BTN_HOLD;
}

bool btn_short_pressed() {
    return btn_data.short_press;
}
