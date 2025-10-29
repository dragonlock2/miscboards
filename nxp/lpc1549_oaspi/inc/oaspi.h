#pragma once

#include <optional>
#include <span>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include "spi.h"

namespace eth {

struct Time; // forward declaration
struct Packet; // forward declaration

class OASPI {
public:
    static constexpr size_t CHUNK_SIZE = 64;

    enum class MMS {
        STANDARD   = 0,
        MAC        = 1,
        PHY_PCS    = 2,
        PHY_PMAPMD = 3,
        PHY_PLCA   = 4,
        VENDOR12   = 12,
    };

    enum class TTSC {
        A = 1, // values match TSC for TX chunk
        B = 2,
        C = 3,
    };

    struct __attribute__((packed)) tx_chunk_t {
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
        std::array<uint8_t, CHUNK_SIZE> data;
    };

    struct __attribute__((packed)) rx_chunk_t {
        std::array<uint8_t, CHUNK_SIZE> data;
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

    using rst_set_callback_t = void (*)(bool assert);

    OASPI(SPI &spi, rst_set_callback_t rst);
    ~OASPI();

    OASPI(OASPI&) = delete;
    OASPI(OASPI&&) = delete;

    static bool parity(std::span<uint8_t, 4> hdr);
    static void fcs_add(Packet &pkt);
    static bool fcs_check(Packet &pkt);

    bool reset();
    void data_transfer(std::span<tx_chunk_t> tx, std::span<rx_chunk_t> rx);

    bool reg_write(MMS mms, uint16_t reg, uint32_t val);
    std::optional<uint32_t> reg_read(MMS mms, uint16_t reg);

    bool mdio_write(uint8_t reg, uint16_t val);
    std::optional<uint16_t> mdio_read(uint8_t reg);
    bool mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val);
    std::optional<uint16_t> mdio_c45_read(uint8_t devad, uint16_t reg);

    // timestamp helpers
    bool ts_enable(bool enable, bool time64); // MUST call until successful
    bool ts_time64();
    std::optional<Time> ts_read(TTSC reg);

protected:
    virtual bool configure() = 0;

    SPI &_spi;
    rst_set_callback_t _rst;
    StaticSemaphore_t _mdio_lock_buffer{};
    SemaphoreHandle_t _mdio_lock{};
    bool _ts_time64;
};

static_assert(sizeof(OASPI::tx_chunk_t) == (OASPI::CHUNK_SIZE + 4));
static_assert(sizeof(OASPI::rx_chunk_t) == (OASPI::CHUNK_SIZE + 4));

#define OASPI_PART(name) \
    class OASPI_##name : public OASPI { \
        using OASPI::OASPI; \
    protected: \
        bool configure() override; \
    }

OASPI_PART(ADIN1110);
OASPI_PART(NCN26010);

};
