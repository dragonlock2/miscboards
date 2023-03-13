#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>
#include <stdint.h>

bool flash_write(uint8_t addr, uint8_t *data, size_t len);
bool flash_read(uint8_t addr, uint8_t *data, size_t len);

#endif // FLASH_H
