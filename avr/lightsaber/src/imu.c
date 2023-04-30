#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "imu.h"

/* private defines */
#define LSM9DS1_ADDR (0x6b)

/* private helpers */
static bool i2c_write(uint8_t addr, const uint8_t *data, size_t len) {
    bool ret = false;
    // start + addr
    TWI0.MADDR = addr << 1;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        goto i2c_write_done;
    }
    // write data bytes
    for (size_t i = 0; i < len; i++) {
        TWI0.MDATA = data[i];
        while (!(TWI0.MSTATUS & TWI_WIF_bm));
        if (TWI0.MSTATUS & TWI_RXACK_bm) {
            goto i2c_write_done;
        }
    }
    ret = true;
i2c_write_done:
    // stop
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    while ((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc);
    return ret;
}

static bool i2c_read(uint8_t addr, uint8_t *data, size_t len) {
    if (len == 0) {
        return false;
    }
    // start + addr
    TWI0.MADDR = addr << 1 | 0x01;
    while (!(TWI0.MSTATUS & (TWI_RIF_bm | TWI_RXACK_bm)));
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        while ((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc);
        return false;
    }
    // read some data bytes
    for (size_t i = 0; i < len - 1; i++) {
        data[i] = TWI0.MDATA;
        TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
        while (!(TWI0.MSTATUS & TWI_RIF_bm));
    }
    // read last data byte + stop
    data[len - 1] = TWI0.MDATA;
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
    while ((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc);
    return true;
}

static inline void lsm9ds1_write_reg(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = { reg, data };
    i2c_write(LSM9DS1_ADDR, buffer, 2);
}

static inline uint8_t lsm9ds1_read_reg(uint8_t reg) {
    uint8_t data = reg;
    i2c_write(LSM9DS1_ADDR, &data, 1);
    i2c_read(LSM9DS1_ADDR, &data, 1);
    return data;
}

/* public functions */
void imu_init() {
    PORTB.DIRSET = PIN0_bm | PIN1_bm;
    TWI0.CTRLA   = TWI_SDASETUP_4CYC_gc | TWI_SDAHOLD_OFF_gc;
    TWI0.MBAUD   = F_CPU / 2 / 400000; // 400kHz
    TWI0.MCTRLA  = TWI_TIMEOUT_DISABLED_gc | TWI_ENABLE_bm;
    TWI0.MCTRLB  = TWI_FLUSH_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    _delay_ms(10);

    lsm9ds1_write_reg(0x1F, 0b00111000); // enable accel XYZ
    lsm9ds1_write_reg(0x20, 0b10100000); // accel 476Hz, 2g range
    lsm9ds1_write_reg(0x21, 0b11000101); // ODR/9 filter
}

void imu_read(imu_data_S *data) {
    uint8_t reg = 0x28;
    i2c_write(LSM9DS1_ADDR, &reg, 1);
    i2c_read(LSM9DS1_ADDR, (uint8_t*) data, 6);
}
