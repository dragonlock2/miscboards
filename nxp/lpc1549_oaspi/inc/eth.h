#pragma once

#include <cstdint>
#include <array>

#define ETH_HDR_LEN (6 + 6 + 2)
#define ETH_MTU     (1500)
#define ETH_MAX_LEN (ETH_HDR_LEN + ETH_MTU + 4) // includes FCS

#define ETH_POOL_SIZE (12)

struct eth_pkt {
    // should make private and add helpers
    std::array<uint8_t, ETH_MAX_LEN> buf;
    size_t len; // excludes header and CRC
};

typedef bool (*eth_callback)(eth_pkt *pkt, void *arg); // return true if taking ownership

void eth_init(void);
eth_pkt *eth_pkt_alloc(bool wait=true);
void eth_pkt_free(eth_pkt *pkt);
void eth_set_cb(size_t idx, eth_callback cb, void *arg);
void eth_send(eth_pkt *pkt); // takes ownership
void eth_get_error(uint32_t &tx_drop, uint32_t &rx_drop);
