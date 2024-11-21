#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <span>
#include <mbedtls/chachapoly.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/sha256.h>
#include <asio.hpp>

struct PubKeys {
    bool odd; // for pub
    std::array<uint8_t, 32> pub;
    std::array<uint8_t, 32> priv;
};

struct SymKeys {
    std::array<uint8_t, 32> host_to_device;
    std::array<uint8_t, 32> device_to_host;
    std::array<uint8_t, 4> confirm;
};
static_assert(sizeof(SymKeys) == 68);

union EncryptedPacket {
    struct {
        std::array<uint8_t, 16> ciphertext;
        std::array<uint8_t, 12> nonce;
        std::array<uint8_t, 4> tag;
        // since guess/check attacks very limited by protocol, short tag ok
        // no good source for this though D:
    };
    std::array<uint8_t, 32> raw;
};
static_assert(sizeof(EncryptedPacket) == 32);

union Payload {
    struct {
        uint8_t next_chan;
    } host;
    struct {
        std::array<uint8_t, 8> hid;
    } device;
    std::array<uint8_t, 8> raw;
};
static_assert(sizeof(Payload) == 8);

class Crypto {
public:
    Crypto();
    ~Crypto();

    void gen_bytes(std::span<uint8_t> buffer);
    PubKeys gen_pubkeys(void);
    std::array<uint8_t, 32> compute_secret(const PubKeys &pair);
    SymKeys compute_symkeys(std::span<const uint8_t, 32> secret);
    void set_keys(std::span<const uint8_t, 32> tx, std::span<const uint8_t, 32> rx);
    EncryptedPacket encrypt(const Payload &data, const EncryptedPacket &prev);
    std::optional<Payload> decrypt(const EncryptedPacket &pkt, const EncryptedPacket *prev);

private:
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng; // used for pubkeys and initial IV
    mbedtls_ecp_group ecdh;
    mbedtls_chachapoly_context cha_tx, cha_rx;
    uint64_t cha_tx_ctr;
};

class Radio {
public:
    Radio(bool host);

    void send(uint8_t chan, uint8_t pipe, std::span<const uint8_t, 32> payload);
    std::optional<uint8_t> recv(uint8_t chan, std::span<uint8_t, 32> buffer);

private:
    bool host;
    asio::io_context ctx;
    asio::ip::udp::socket tx, rx;
    std::optional<uint8_t> rx_chan;
};

class Protocol {
public:
    Protocol(Crypto &crypt, Radio &rf);

    SymKeys pair(uint32_t retry_ms);
    std::optional<uint8_t> poll(uint8_t chan, std::span<uint8_t, 32> buffer, uint32_t timeout_ms);

private:
    Crypto &crypt;
    Radio &rf;
};
