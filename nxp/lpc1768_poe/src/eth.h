#pragma once

void eth_init(void);
void eth_mdio_write(uint8_t reg, uint16_t val);
uint16_t eth_mdio_read(uint8_t reg);
