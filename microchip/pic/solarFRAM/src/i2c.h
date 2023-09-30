#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void i2c_init(void);
bool i2c_write(uint8_t addr, uint8_t *data, size_t len);
bool i2c_read(uint8_t addr, uint8_t *data, size_t len);

#endif // I2C_H
