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

    Packet(void) : _len(0) {}

    std::span<uint8_t> raw(void) { return std::span<uint8_t>(_buf.data(), HDR_LEN + _len + 4); }
    void set_len(size_t len) { _len = len; }

private:
    friend class Eth;

    std::array<uint8_t, MAX_LEN> _buf;
    size_t _len; // excludes header and CRC
};

class Eth {
public:
    typedef bool (*rx_callback)(Packet *pkt, void *arg); // return true if taking ownership

    // matches if substring of "Hardware Port" from "networksetup -listallhardwareports"
    // if iface (ex. "en0") specified, ignores name and matches iface exactly
    Eth(const char *name="lpc1549_oaspi", const char *iface=nullptr);
    ~Eth();

    Eth(Eth&) = delete;
    Eth(Eth&&) = delete;

    static Packet *pkt_alloc(bool wait=true);
    static void pkt_free(Packet *pkt);

    void set_cb(size_t idx, rx_callback cb, void *arg);
    void send(Packet *pkt); // takes ownership

private:
    pcap_t *dev;
    struct {
        std::mutex lock;
    } tx;
    struct {
        std::thread thread;
        std::mutex cbs_lock;
        std::array<std::tuple<rx_callback, void*>, 8> cbs;
        bool run;
    } rx;
};

};
