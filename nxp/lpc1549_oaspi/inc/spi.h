#pragma once

#include <cstddef>
#include <cstdint>

void spi_init(void);
void spi_transceive(uint8_t* tx, uint8_t* rx, size_t len);
