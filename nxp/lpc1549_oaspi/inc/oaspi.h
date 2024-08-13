#pragma once

enum class oaspi_mms {
    STANDARD = 0x00,
    MAC      = 0x01,
};

void oaspi_init(void);
void oaspi_reg_write(oaspi_mms mms, uint16_t reg, uint32_t val);
uint32_t oaspi_reg_read(oaspi_mms mms, uint16_t reg);
void oaspi_mdio_write(uint8_t reg, uint16_t val);
uint16_t oaspi_mdio_read(uint8_t reg);
void oaspi_mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val);
uint16_t oaspi_mdio_c45_read(uint8_t devad, uint16_t reg);
