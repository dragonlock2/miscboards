#include <stdbool.h>
#include <project.h>
#include "dbg.h"
#include "mdio_bb.h"

/* private defines */
#define MDIO_DELAY (1) // us (half period)

/* private helpers */
// heavily inspired by Linux mdio-bitbang.c
static inline void mdio_write_bit(bool b) {
    MDIO_Write(b);
    CyDelayUs(MDIO_DELAY);
    MDC_Write(1);
    CyDelayUs(MDIO_DELAY);
    MDC_Write(0);
}

static inline bool mdio_read_bit() {
    MDIO_Write(1); // "release" bus
    CyDelayUs(MDIO_DELAY);
    MDC_Write(1);
    CyDelayUs(MDIO_DELAY);
    MDC_Write(0);
    return MDIO_Read();
}

static inline void mdio_write_num(uint32_t val, size_t num) {
    for (int i = num - 1; i >= 0; i--) {
        mdio_write_bit((val >> i) & 0x1);
    }
}

static inline uint32_t mdio_read_num(size_t num) {
    uint32_t val = 0;
    for (int i = num - 1; i >= 0; i--) {
        val <<= 1;
        val |= mdio_read_bit();
    }
    return val;
}

static inline void mdio_cmd(uint8_t addr, uint8_t reg, bool write) {
    for (int i = 0; i < 32; i++) {
        mdio_write_bit(1);
    }
    mdio_write_bit(0);
    mdio_write_bit(1);
    mdio_write_bit(!write);
    mdio_write_bit(write);
    mdio_write_num(addr, 5);
    mdio_write_num(reg, 5);
}

/* public functions */
void mdio_init() {
    MDIO_Write(1);
    MDC_Write(0);
}

void mdio_write_reg(uint8_t addr, uint8_t reg, uint16_t data) {
    mdio_cmd(addr, reg, true);
    mdio_write_bit(1);
    mdio_write_bit(0);
    mdio_write_num(data, 16);
    mdio_read_bit(); // dummy read
}

uint16_t mdio_read_reg(uint8_t addr, uint8_t reg) {
    mdio_cmd(addr, reg, false);
    if (mdio_read_bit() != 0) { // no "ack"
        for (int i = 0; i < 32; i++) {
            mdio_read_bit();
        }
        return 0xFFFF;
    }
    uint16_t data = mdio_read_num(16);
    mdio_read_bit(); // dummy read
    return data;
}
