#pragma once

// macOS implementation of Eth class

#include <mutex>
#include <span>
#include <thread>
#include <pcap.h>

namespace eth {

struct Packet {
    static constexpr size_t HDR_LEN = 6 + 6 + 2;
    static constexpr size_t MTU     = 1500;
    static constexpr size_t MAX_LEN = HDR_LEN + MTU + 4; // includes FCS

    Packet() : _len(0) {}

    std::span<uint8_t> raw() { return std::span<uint8_t>(_buf.data(), HDR_LEN + _len + 4); }
    void set_len(size_t len) { _len = len; }

private:
    friend class Eth;

    std::array<uint8_t, MAX_LEN> _buf;
    size_t _len; // excludes header and CRC
};

class Eth {
public:
    using tx_callback = void (*)();
    using rx_callback = bool (*)(Packet *pkt, void *arg); // return true if taking ownership

    // matches if substring of "Hardware Port" from "networksetup -listallhardwareports"
    // if iface (ex. "en0") specified, ignores name and matches iface exactly
    Eth(const char *name="10BASE-T1S", const char *iface=nullptr);
    ~Eth();

    Eth(Eth&) = delete;
    Eth(Eth&&) = delete;

    Packet *pkt_alloc(bool wait=true);
    void pkt_free(Packet *pkt);

    void set_tx_cb(tx_callback cb);
    void set_rx_cb(rx_callback cb, void *arg);
    bool send(Packet *pkt, bool wait=true); // always takes ownership

private:
    pcap_t *_dev{}, *_tx_dev{};
    struct {
        std::mutex lock;
        tx_callback cb;
    } _tx{};
    struct {
        std::thread thread;
        std::mutex cb_lock;
        std::tuple<rx_callback, void*> cb;
        bool run;
    } _rx{};
};

};
