#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void flash_init();
void flash_set_read_addr(uint32_t addr);
uint8_t flash_read_byte();

#endif // FLASH_H
