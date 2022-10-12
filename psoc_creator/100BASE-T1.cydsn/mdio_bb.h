#ifndef MDIO_BB_H
#define MDIO_BB_H

#include <stdint.h>

void mdio_init();
void mdio_write_reg(uint8_t addr, uint8_t reg, uint16_t data);
uint16_t mdio_read_reg(uint8_t addr, uint8_t reg);

#endif // MDIO_BB_H
