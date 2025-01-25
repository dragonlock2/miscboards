#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <tuple>
#include "oaspi.h"

namespace eth {

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

class Eth {
public:
    typedef void (*int_callback)(void *arg);
    typedef void (*int_set_callback)(int_callback cb, void *arg);
    typedef void (*tx_callback)(void);
    typedef bool (*rx_callback)(Packet *pkt, void *arg); // return true if taking ownership

    static constexpr size_t POOL_SIZE = 10; // global pool
    static constexpr size_t REQ_SIZE  = POOL_SIZE / 2; // can adjust if >1 instance

    Eth(OASPI &oaspi, int_set_callback int_cb);
    ~Eth();

    Eth(Eth&) = delete;
    Eth(Eth&&) = delete;

    Packet *pkt_alloc(bool wait=true);
    void pkt_free(Packet *pkt);

    void set_tx_cb(tx_callback cb);
    void set_rx_cb(rx_callback cb, void *arg);
    bool send(Packet *pkt, bool wait=true); // always takes ownership
    std::tuple<uint32_t, uint32_t> get_error(void); // tx drop, rx drop

private:
    struct Helper;

    OASPI &oaspi;
    int_set_callback int_set_cb;
    TaskHandle_t handle;
    struct {
        SemaphoreHandle_t lock;
        tx_callback tx_cb;
        std::tuple<rx_callback, void*> rx_cb;
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
