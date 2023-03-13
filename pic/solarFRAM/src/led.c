#include <xc.h>
#include "led.h"

void led_off(void) {
    TRISA2 = 1; // @ 5V, not fully off :(
}

void led_write(bool pass) {
    LATA2  = pass;
    TRISA2 = 0;
}
