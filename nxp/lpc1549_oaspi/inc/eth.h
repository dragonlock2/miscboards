#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <tuple>
#include "oaspi.h"

namespace eth {

constexpr size_t POOL_SIZE = 12; // global pool

struct Packet {
    static constexpr size_t HDR_LEN = 6 + 6 + 2;
    static constexpr size_t MTU     = 1500;
    static constexpr size_t MAX_LEN = HDR_LEN + MTU + 4; // includes FCS

    std::span<uint8_t> raw(void) { return std::span<uint8_t>(_buf.data(), HDR_LEN + _len + 4); }
    void set_len(size_t len) { _len = len; }

private:
    friend class Eth;

    std::array<uint8_t, MAX_LEN> _buf;
    size_t _len; // excludes header and CRC
};

typedef void (*int_callback)(void *arg);
typedef void (*int_set_callback)(int_callback cb, void *arg);
typedef bool (*rx_callback)(Packet *pkt, void *arg); // return true if taking ownership

class Eth {
public:
    Eth(OASPI &oaspi, int_set_callback cb);
    ~Eth();

    Eth(Eth&) = delete;
    Eth(Eth&&) = delete;

    static Packet *pkt_alloc(bool wait=true);
    static void pkt_free(Packet *pkt);

    void set_cb(size_t idx, rx_callback cb, void *arg);
    void send(Packet *pkt); // takes ownership
    std::tuple<uint32_t, uint32_t> get_error(void); // tx drop, rx drop

private:
    struct Helper;

    OASPI &oaspi;
    int_set_callback int_set_cb;
    TaskHandle_t handle;
    struct {
        SemaphoreHandle_t lock;
        std::array<std::tuple<rx_callback, void*>, 2> cbs;
    } callbacks;
    struct {
        QueueHandle_t reqs;
        bool start;
        Packet *pkt;
        size_t idx, len;
        oaspi_tx_chunk chunk;
        uint32_t drops;
    } tx;
    struct {
        Packet *pkt;
        size_t len;
        oaspi_rx_chunk chunk;
        uint32_t drops;
    } rx;
};

};
