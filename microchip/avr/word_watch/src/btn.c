#include <avr/io.h>
#include "btn.h"

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
