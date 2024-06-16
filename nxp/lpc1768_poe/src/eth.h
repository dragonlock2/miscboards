#pragma once

void eth_init(void);
void eth_mdio_write(uint8_t reg, uint16_t val);
uint16_t eth_mdio_read(uint8_t reg);
bool eth_link_up(void);
void eth_write(uint8_t* buffer, size_t len);
bool eth_read(uint8_t*& buffer, size_t& len);
void eth_read_done(void);
