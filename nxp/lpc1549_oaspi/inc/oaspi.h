#pragma once

#include <array>
#include <cstdint>

#define OASPI_MAX_PKT_LEN (1518)

enum class oaspi_mms {
    STANDARD   = 0,
    MAC        = 1,
    PHY_PCS    = 2,
    PHY_PMAPMD = 3,
    PHY_PLCA   = 4,
    VENDOR12   = 12,
};

struct __attribute__((packed)) oaspi_tx_chunk {
    union {
        struct {
            uint8_t RSVD0 : 5;
            uint8_t NORX  : 1;
            uint8_t SEQ   : 1;
            uint8_t DNC   : 1;
            uint8_t SWO   : 4;
            uint8_t SV    : 1;
            uint8_t DV    : 1;
            uint8_t VS    : 2;
            uint8_t EBO   : 6;
            uint8_t EV    : 1;
            uint8_t RSVD1 : 1;
            uint8_t P     : 1;
            uint8_t RSVD2 : 5;
            uint8_t TSC   : 2;
        };
        std::array<uint8_t, 4> header;
    };
    std::array<uint8_t, 64> data;
};

struct __attribute__((packed)) oaspi_rx_chunk {
    std::array<uint8_t, 64> data;
    union {
        struct {
            uint8_t RCA  : 5;
            uint8_t SYNC : 1;
            uint8_t HDRB : 1;
            uint8_t EXST : 1;
            uint8_t SWO  : 4;
            uint8_t SV   : 1;
            uint8_t DV   : 1;
            uint8_t VS   : 2;
            uint8_t EBO  : 6;
            uint8_t EV   : 1;
            uint8_t FD   : 1;
            uint8_t P    : 1;
            uint8_t TXC  : 5;
            uint8_t RTSP : 1;
            uint8_t RTSA : 1;
        };
        std::array<uint8_t, 4> footer;
    };
};

static_assert(sizeof(oaspi_tx_chunk) == 68);
static_assert(sizeof(oaspi_rx_chunk) == 68);

void oaspi_init(void);
void oaspi_configure(void);

bool oaspi_parity(std::array<uint8_t, 4>& hdr);
void oaspi_fcs_add(std::array<uint8_t, OASPI_MAX_PKT_LEN>& pkt, size_t len);
bool oaspi_fcs_check(std::array<uint8_t, OASPI_MAX_PKT_LEN>& pkt, size_t len);
void oaspi_data_transfer(oaspi_tx_chunk& tx, oaspi_rx_chunk& rx);

void oaspi_reg_write(oaspi_mms mms, uint16_t reg, uint32_t val);
uint32_t oaspi_reg_read(oaspi_mms mms, uint16_t reg);
void oaspi_mdio_write(uint8_t reg, uint16_t val);
uint16_t oaspi_mdio_read(uint8_t reg);
void oaspi_mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val);
uint16_t oaspi_mdio_c45_read(uint8_t devad, uint16_t reg);
