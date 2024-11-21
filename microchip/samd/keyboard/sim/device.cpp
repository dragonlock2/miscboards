#include <chrono>
#include <iostream>
#include <thread>
#include "common.h"

using namespace std::chrono_literals;

// save in flash
bool key_valid = true;
std::array<uint8_t, 32> key_tx { 0x04, 0x8d, 0xe2, 0x1a, 0x00, 0x9b, 0x2c, 0xee, 0x6c, 0xbd, 0x34, 0x99, 0x9d, 0x16, 0xb5, 0xc0, 0x3e, 0x4e, 0x5e, 0x56, 0xa1, 0xb2, 0x72, 0x54, 0xbb, 0xd9, 0xea, 0xb8, 0xf4, 0x86, 0x0f, 0x12, };
std::array<uint8_t, 32> key_rx { 0xfd, 0x5d, 0xb4, 0xdb, 0x3d, 0xea, 0x9e, 0x17, 0x3e, 0xeb, 0x02, 0x0a, 0x66, 0xd5, 0x70, 0xa4, 0xeb, 0x90, 0xfb, 0xee, 0xbf, 0x27, 0x8c, 0xab, 0xaf, 0xdc, 0x89, 0x2b, 0x8f, 0x51, 0x61, 0xd4, };

int main(void) {
    Crypto crypt;
    Radio rf(false);
    Protocol prot(crypt, rf);

    // pair if needed
    while (!key_valid) {
        auto keys = prot.pair(13);
        auto &c = keys.confirm;
        auto confirm = std::format("{:02x}{:02x}{:02x}{:02x}", c[0], c[1], c[2], c[3]);
        printf("type confirmation code: ");
        std::string usr_confirm;
        std::getline(std::cin, usr_confirm);
        if (confirm == usr_confirm) {
            key_valid = true;
            key_tx = keys.device_to_host;
            key_rx = keys.host_to_device;
            printf("tx: { "); for (auto &b: key_tx) { printf("0x%02x, ", b); } printf("};\r\n");
            printf("rx: { "); for (auto &b: key_rx) { printf("0x%02x, ", b); } printf("};\r\n");
        } else {
            printf("confirmation mismatch, restarting pairing...\r\n");
        }
    }
    crypt.set_keys(key_tx, key_rx);

    // initialize vars
    EncryptedPacket tx_pkt, rx_pkt;
    Payload tx_pl, rx_pl;
    uint8_t curr_chan;
    crypt.gen_bytes(rx_pkt.raw);
    tx_pl.raw.fill(0);
    crypt.gen_bytes(std::span<uint8_t>(&curr_chan, 1));

    // runtime
    while (true) {
        // construct TX packet
        tx_pkt = crypt.encrypt(tx_pl, rx_pkt);

        // send TX packet and listen for valid RX
        bool found = false;
        while (!found) {
            rf.send(curr_chan, 0, tx_pkt.raw);
            auto pipe = prot.poll(curr_chan, rx_pkt.raw, 2);
            if (!pipe.has_value() || pipe.value() != 0) {
                continue;
            }
            auto pl = crypt.decrypt(rx_pkt, &tx_pkt);
            if (pl.has_value()) {
                rx_pl = pl.value();
                found = true;
            }
        }
        curr_chan = rx_pl.host.next_chan;

        // wait for user input
        char c = getchar();
        while (c == 10) {
            c = getchar(); // ignore enter
        }
        tx_pl.device.hid[0] = c;
    }
}
