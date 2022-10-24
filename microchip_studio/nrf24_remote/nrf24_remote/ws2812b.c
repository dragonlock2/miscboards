#define F_CPU 20000000L

#include <util/delay.h>
#include "mcc_generated_files/system/system.h"
#include "ws2812b.h"

/* private helpers */
static inline void ws2812b_write_one() {
    // tuned for 20MHz
    led_SetHigh();
    _delay_loop_1(5);
    led_SetLow();
    _delay_loop_1(2);
}

static inline void ws2812b_write_zero() {
    // tuned for 20MHz
    led_SetHigh();
    _delay_loop_1(2);
    led_SetLow();
    _delay_loop_1(5);
}

static inline void ws2812b_write_byte(uint8_t b) {
    for (int i = 0; i < 8; i++) {
        if (b & 0x80) {
            ws2812b_write_one();
        } else {
            ws2812b_write_zero();
        }
        b <<= 1;
    }
}

/* public functions */
void ws2812b_write(uint8_t r, uint8_t g, uint8_t b) {
    led_SetLow();
    ws2812b_write_byte(g);
    ws2812b_write_byte(r);
    ws2812b_write_byte(b);
    _delay_us(50);
}
