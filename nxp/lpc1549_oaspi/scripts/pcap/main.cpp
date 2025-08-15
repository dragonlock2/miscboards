#include <chrono>
#include <cstdio>
#include "eth.h"

using namespace std::chrono_literals;

static bool cb(eth::Packet *pkt, void*) {
    static auto start = std::chrono::high_resolution_clock::now();
    auto dt  = std::chrono::high_resolution_clock::now() - start;
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
    auto raw = pkt->raw();
    std::printf("rx %lu-bytes @ %llums\r\n", raw.size() - 4, ms);
    return false;
}

int main() {
    eth::Eth dev;
    dev.set_rx_cb(cb, nullptr);

    std::array<uint8_t, 6> src {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x69};
    std::array<uint8_t, 6> dst {0x98, 0xED, 0x5C, 0x00, 0x00, 0x69};
    for (auto i = 0; i < 8; i++) {
        auto pkt = dev.pkt_alloc();
        std::memcpy(pkt->raw().subspan(6).data(), src.data(), 6);
        std::memcpy(pkt->raw().subspan(0).data(), dst.data(), 6);
        pkt->raw()[12] = 0x42;
        pkt->raw()[13] = 0x69;
        pkt->raw()[14] = i;
        pkt->set_len(1);
        dev.send(pkt);
        std::this_thread::sleep_for(10ms);
    }

    std::this_thread::sleep_for(3000ms); // listen for packets
}
