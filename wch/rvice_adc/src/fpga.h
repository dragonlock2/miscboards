#ifndef FPGA_H
#define FPGA_H

#include <array>
#include <cstdbool>
#include <cstdint>

constexpr size_t SECTOR_SIZE = 4096;
constexpr size_t PAGE_SIZE = 256;

void fpga_init(void);
void fpga_off(void);
void fpga_on(void);
bool fpga_booted(void);

bool fpga_erase(uint32_t addr, size_t len);
bool fpga_write(uint32_t addr, const std::array<uint8_t, PAGE_SIZE> &data);
bool fpga_read(uint32_t addr, std::array<uint8_t, PAGE_SIZE> &data);

#endif // FPGA_H
