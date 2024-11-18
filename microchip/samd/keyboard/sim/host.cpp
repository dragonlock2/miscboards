#include <chrono>
#include <iostream>
#include <thread>
#include "common.h"

using namespace std::chrono_literals;

// save in flash
bool key_valid = false;
std::array<uint8_t, 32> key_tx {};
std::array<uint8_t, 32> key_rx {};

int main(void) {
    Crypto crypt;
    Radio rf(true);
    Protocol prot(crypt, rf);

    // pair if needed
    while (!key_valid) {
        auto keys = prot.pair(11);
        auto &c = keys.confirm;
        printf("confirmation code: %02x%02x%02x%02x\r\n", c[0], c[1], c[2], c[3]);
        printf("type \"yes\" to confirm: ");
        std::string confirm;
        std::getline(std::cin, confirm);
        if (confirm == "yes") {
            key_valid = true;
            key_tx = keys.host_to_device;
            key_rx = keys.device_to_host;
            printf("tx: { "); for (auto &b: key_tx) { printf("0x%02x, ", b); } printf("};\r\n");
            printf("rx: { "); for (auto &b: key_rx) { printf("0x%02x, ", b); } printf("};\r\n");
        } else {
            printf("restarting pairing...\r\n");
        }
    }
    crypt.set_keys(key_tx, key_rx);

    // TODO sync
    // should unpair after enough bad tags

    // TODO runtime
    // make sure initial prev received packet is random
}
