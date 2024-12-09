#pragma once

#include <cstddef>
#include <cstdint>

class SPI {
public:
    SPI(void);
    ~SPI();

    SPI(SPI&) = delete;
    SPI(SPI&&) = delete;

    void transceive(uint8_t *tx, uint8_t *rx, size_t len);
};
