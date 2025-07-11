#pragma once

#include <span>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include "spi.h"

namespace eth {

struct Packet; // forward declaration

class OASPI {
public:
    enum class MMS {
        STANDARD   = 0,
        MAC        = 1,
        PHY_PCS    = 2,
        PHY_PMAPMD = 3,
        PHY_PLCA   = 4,
        VENDOR12   = 12,
    };

    struct __attribute__((packed)) tx_chunk {
        union {
            // using uint8_t is messier but should be endian agnostic
            // MSVC, GCC, and Clang order bitfields from low to high
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

    struct __attribute__((packed)) rx_chunk {
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

    typedef void (*rst_set_callback)(bool assert);

    OASPI(SPI &spi, rst_set_callback rst);
    ~OASPI();

    OASPI(OASPI&) = delete;
    OASPI(OASPI&&) = delete;

    static bool parity(std::span<uint8_t, 4> hdr);
    static void fcs_add(Packet &pkt);
    static bool fcs_check(Packet &pkt);

    void reset(void);
    void data_transfer(tx_chunk &tx, rx_chunk &rx);

    void reg_write(MMS mms, uint16_t reg, uint32_t val);
    uint32_t reg_read(MMS mms, uint16_t reg);
    void mdio_write(uint8_t reg, uint16_t val);
    uint16_t mdio_read(uint8_t reg);
    void mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val);
    uint16_t mdio_c45_read(uint8_t devad, uint16_t reg);

protected:
    virtual void configure(void) = 0;

    SPI &spi;
    rst_set_callback rst;
    StaticSemaphore_t mdio_lock_buffer;
    SemaphoreHandle_t mdio_lock;
};

static_assert(sizeof(OASPI::tx_chunk) == 68);
static_assert(sizeof(OASPI::rx_chunk) == 68);

#define OASPI_PART(name) \
    class OASPI_##name : public OASPI { \
        using OASPI::OASPI; \
    protected: \
        void configure(void) override; \
    }

OASPI_PART(ADIN1110);
OASPI_PART(NCN26010);

};
