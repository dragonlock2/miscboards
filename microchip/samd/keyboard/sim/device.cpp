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

    // TODO runtime
}
