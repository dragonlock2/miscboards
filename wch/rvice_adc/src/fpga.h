#ifndef FPGA_H
#define FPGA_H

#include <array>
#include <cstdbool>
#include <cstdint>

void fpga_init(void);
void fpga_off(void);
void fpga_on(void);
bool fpga_booted(void);

void fpga_erase(uint32_t addr, size_t len); // note 4KiB sectors
void fpga_write(uint32_t addr, const uint8_t data[256]);
void fpga_read(uint32_t addr, uint8_t data[256]);

#endif // FPGA_H
