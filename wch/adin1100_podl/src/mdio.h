#pragma once

#include <cstdint>

#define PHY_ADIN1100 (0b00100)
#define PHY_RTL8201F (0b00000)

void mdio_write(uint8_t phy, uint8_t reg, uint16_t val);
uint16_t mdio_read(uint8_t phy, uint8_t reg);

void mdio_write_c45(uint8_t phy, uint8_t devad, uint16_t reg, uint16_t val);
uint16_t mdio_read_c45(uint8_t phy, uint8_t devad, uint16_t reg);
