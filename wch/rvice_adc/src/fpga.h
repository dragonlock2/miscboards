#ifndef FPGA_H
#define FPGA_H

#include <array>
#include <cstdbool>
#include <cstdint>

void fpga_off(void);
void fpga_on(void);
bool fpga_booted(void);

bool fpga_erase(uint32_t addr, size_t len);
bool fpga_write(uint32_t addr, const uint8_t data[256]);

#endif // FPGA_H
