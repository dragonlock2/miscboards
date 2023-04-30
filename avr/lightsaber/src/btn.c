#include <avr/io.h>
#include "btn.h"

/* private defines */
#define BTN_PORT (PORTA)
#define BTN_PIN  (PIN5_bm)
#define BTN_CTRL (PORTA.PIN5CTRL)

/* private data */
struct {
    bool prev;
    bool rising;
    bool falling;
} btn_data;

/* public functions */
void btn_init() {
    BTN_PORT.DIRCLR = BTN_PIN;
    BTN_CTRL = 0x88; // INVEN=1, PULLUPEN=1
}

void btn_run50Hz() {
    bool curr = BTN_PORT.IN & BTN_PIN;
    btn_data.rising  = curr  && !btn_data.prev;
    btn_data.falling = !curr && btn_data.prev;
    btn_data.prev    = curr;
}

bool btn_rising() {
    return btn_data.rising;
}

bool btn_falling() {
    return btn_data.falling;
}
