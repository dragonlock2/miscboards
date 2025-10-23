#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <tuple>
#include "oaspi.h"

/*
 * If CONFIG_ETH_MIN_LATENCY is defined, driver optimizes for best case minimum latency.
 * - assumes uninterrupted >21MHz SPI transfers (ex. DMA)
 * - higher CPU cost due to extra task switching and interrupt overhead
 * - higher RAM cost due to contiguous transfer needed for transmit
 *
 * Without CONFIG_ETH_MIN_LATENCY defined, can expect the following rough latency numbers.
 * - best case latency is number of chunks needed for packet
 * - worst case latency is MAX_CHUNKS plus chunks for packet (TX during RX and vice versa)
 *
 * With CONFIG_ETH_MIN_LATENCY defined, can expect the following rough latency numbers.
 * - best case latency is ~0
 * - worst case TX latency is ~1 chunk over SPI (when TX request during RX)
 * - worst case RX latency is ~24 chunks over SPI (when RX during max length TX)
 *
 * There are many possible ways to determine how many chunks to process at a time which all
 * impact CPU/RAM cost and latency in various load scenarios. The best choice for you depends.
 * CONFIG_ETH_MIN_LATENCY sacrifices worst case RX latency for better best and likely average
 * case latency at higher CPU/RAM cost. Without CONFIG_ETH_MIN_LATENCY, we scale between lower
 * CPU cost and higher RAM cost based on MAX_CHUNKS. If you truly want zero latency, just use
 * a PHY and an internal MAC peripheral.
 *
 * Measurements for 1 packet round-trip improvement.
 * - 64-byte: 520us => 503us
 * - 65-byte: 540us => 519us
 * - 1518-byte: 7216us => 5711us
 *
 * Measurements for 2 packet round-trip improvement.
 * - 64-byte: 656us => 636us
 * - 65-byte: 677us => 653us
 * - 1518-byte: 8742us => 7317us
 */

namespace eth {

struct Packet {
    static constexpr size_t HDR_LEN = 6 + 6 + 2;
    static constexpr size_t MTU     = 1500;
    static constexpr size_t MAX_LEN = HDR_LEN + MTU + 4; // includes FCS

    std::span<uint8_t> raw() { return std::span<uint8_t>(_buf.data(), HDR_LEN + _len + 4); }
    void set_len(size_t len) { _len = len; }

    std::optional<uint32_t> timestamp_id; // for TX, specifies timestamp capture desired
    std::optional<std::tuple<uint32_t, uint32_t>> timestamp; // for RX, specifies timestamp as (seconds, nanoseconds)

private:
    friend class Eth;

    std::array<uint8_t, MAX_LEN> _buf;
    size_t _len; // excludes header and CRC
};

class Eth {
public:
    using int_callback     = void (*)(void *arg);
    using int_set_callback = void (*)(int_callback cb, void *arg);
    using tx_callback      = void (*)();
    using rx_callback      = bool (*)(Packet *pkt, void *arg); // return true if taking ownership

    static constexpr size_t POOL_SIZE  = 8; // global pool
    static constexpr size_t REQ_SIZE   = POOL_SIZE / 2; // can adjust if >1 instance
#ifdef CONFIG_ETH_MIN_LATENCY
    static constexpr size_t MAX_CHUNKS = (Packet::MAX_LEN + OASPI::CHUNK_SIZE - 1) / OASPI::CHUNK_SIZE;
#else
    static constexpr size_t MAX_CHUNKS = 8;
#endif

    Eth(OASPI &oaspi, int_set_callback int_set);
    ~Eth();

    Eth(Eth&) = delete;
    Eth(Eth&&) = delete;

    Packet *pkt_alloc(bool wait=true);
    void pkt_free(Packet *pkt);

    void set_tx_cb(tx_callback cb);
    void set_rx_cb(rx_callback cb, void *arg);
    bool send(Packet *pkt, bool wait=true); // always takes ownership

    std::tuple<uint32_t, uint32_t> get_drops(); // tx, rx
    std::tuple<uint32_t, uint32_t> get_total(); // tx, rx
    bool error();

    std::optional<std::tuple<uint32_t, uint32_t>> timestamp_read(uint32_t id);

private:
    struct Helper;

    OASPI &_oaspi;
    int_set_callback _int_set;
    struct {
        StaticTask_t buffer;
        std::array<StackType_t, configETH_STACK_SIZE> stack;
        TaskHandle_t handle;
        bool error;
    } _task{};
    struct {
        StaticSemaphore_t lock_buffer;
        SemaphoreHandle_t lock;
        tx_callback tx_cb;
        std::tuple<rx_callback, void*> rx_cb;
    } _callbacks{};
    struct {
        std::array<Packet*, REQ_SIZE> reqs_buf;
        StaticQueue_t reqs_data;
        QueueHandle_t reqs;
        Packet *pkt;
        bool start;
        size_t idx, len;
        std::array<OASPI::tx_chunk, MAX_CHUNKS> chunks;
        size_t free_chunks;
        uint32_t total, drops;
    } _tx{};
    struct {
        Packet *pkt;
        bool ts_expect, ts_parity;
        size_t len;
        std::array<OASPI::rx_chunk, MAX_CHUNKS> chunks;
        size_t pend_chunks;
        uint32_t total, drops;
    } _rx{};
};

};
