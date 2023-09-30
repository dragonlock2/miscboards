#include <xc.h>
#include "i2c.h"

// based on http://www.bitbanging.space/posts/bitbang-i2c

/* HAL */
static inline void hal_init(void) {
    TRISA4 = 0;
    TRISA5 = 0;
    ODA4   = 1;
    ODA5   = 1;
    ANSA4  = 0;
}

static inline void sda_low(void)  { LATA4 = 0; }
static inline void sda_high(void) { LATA4 = 1; }
static inline bool sda_read(void) { return RA4; }

static inline void scl_low(void)  { LATA5 = 0; }
static inline void scl_high(void) { LATA5 = 1; }
static inline bool scl_read(void) { return RA5; }

static inline void clk_delay(void) {}

/* private helpers */
static void i2c_start(void) {
    sda_high();
    clk_delay();
    scl_high();
    clk_delay();
    sda_low();
    clk_delay();
    scl_low();
    clk_delay();
}

static void i2c_stop(void) {
    sda_low();
    clk_delay();
    scl_high();
    clk_delay();
    sda_high();
    clk_delay();
}

static bool i2c_write_byte(uint8_t c) {
    for (uint8_t i = 0; i < 8; i++) {
        (c & 0x80) ? sda_high() : sda_low();
        clk_delay();
        scl_high();
        clk_delay();
        scl_low();
        clk_delay();
        c <<= 1;
    }
    sda_high();
    scl_high();
    clk_delay();
    bool ack = !sda_read();
    scl_low();
    clk_delay();
    return ack;
}

static uint8_t i2c_read_byte(bool ack) {
    uint8_t c = 0;
    sda_high();
    for (uint8_t i = 0; i < 8; i++) {
        c <<= 1;
        scl_high();
        while (scl_read() == 0); // clock stretching
        clk_delay();
        c |= sda_read();
        scl_low();
        clk_delay();
    }
    ack ? sda_low() : sda_high();
    scl_high();
    clk_delay();
    scl_low();
    sda_high();
    return c;
}

/* public functions */
void i2c_init(void) {
    hal_init();
    sda_high();
    scl_high();
}

bool i2c_write(uint8_t addr, uint8_t *data, size_t len) {
    bool ret = false;
    i2c_start();
    if (!i2c_write_byte((uint8_t)(addr << 1) | 0x00)) { goto i2c_write_done; }
    for (size_t i = 0; i < len; i++) {
        if (!i2c_write_byte(data[i])) { goto i2c_write_done; }
    }
    ret = true;
i2c_write_done:
    i2c_stop();
    return ret;
}

bool i2c_read(uint8_t addr, uint8_t *data, size_t len) {
    bool ret = false;
    i2c_start();
    if (!i2c_write_byte((uint8_t)(addr << 1) | 0x01)) { goto i2c_read_done; }
    for (size_t i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i < len - 1); // nack last byte
    }
    ret = true;
i2c_read_done:
    i2c_stop();
    return ret;
}
