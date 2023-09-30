#include <string.h>
#include "i2c.h"
#include "flash.h"

#define FM24C04B_ADDR (0b01010000) // only using page 0 (256 bytes)

static uint8_t buffer[16]; // no VLA :(

bool flash_write(uint8_t addr, uint8_t *data, size_t len) {
    buffer[0] = addr;
    memcpy(buffer + 1, data, len);
    return i2c_write(FM24C04B_ADDR, buffer, len + 1);
}

bool flash_read(uint8_t addr, uint8_t *data, size_t len) {
    buffer[0] = addr;
    if (!i2c_write(FM24C04B_ADDR, buffer, 1)) { return false; }
    return i2c_read(FM24C04B_ADDR, data, len);
}
