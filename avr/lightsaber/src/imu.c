#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "imu.h"

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

/* public functions */
void imu_init() {
    PORTB.DIRSET = PIN0_bm | PIN1_bm;
    TWI0.CTRLA   = TWI_SDASETUP_4CYC_gc | TWI_SDAHOLD_OFF_gc;
    TWI0.MBAUD   = F_CPU / 2 / 400000; // 400kHz
    TWI0.MCTRLA  = TWI_TIMEOUT_DISABLED_gc | TWI_ENABLE_bm;
    TWI0.MCTRLB  = TWI_FLUSH_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    _delay_ms(1);

    // TODO lsm9ds1 setup
}
