#include <avr/io.h>
#include "btn.h"

/* private data */
typedef struct {
    bool prev;
    bool rising;
} btn_data_S;
static btn_data_S btn_data;

/* public functions */
void btn_init() {
    PORTC.DIRCLR = PIN5_bm;
    PORTC.PIN5CTRL = 0x88; // INVEN=1, PULLUPEN=1
}

void btn_run50Hz() {
    bool curr = PORTC.IN & PIN5_bm;
    btn_data.rising = curr & !btn_data.prev;
    btn_data.prev = curr;
}

bool btn_rising() {
    return btn_data.rising;
}
