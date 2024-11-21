#include <chrono>
#include <thread>
#include "common.h"

extern "C" {
#include <mbedtls/constant_time.h> // mbedtls forgot...
};

using namespace std::chrono_literals;

Crypto::Crypto() {
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&rng);
    mbedtls_ecp_group_init(&ecdh);
    mbedtls_chachapoly_init(&cha_tx);
    mbedtls_chachapoly_init(&cha_rx);

    // make sure to add several entropy sources (not just ADC)
    assert(mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0) == 0);
    assert(mbedtls_ecp_group_load(&ecdh, MBEDTLS_ECP_DP_SECP256R1) == 0);
    assert(mbedtls_ecdh_can_do(MBEDTLS_ECP_DP_SECP256R1) == 1);
}

Crypto::~Crypto() {
    mbedtls_chachapoly_free(&cha_rx);
    mbedtls_chachapoly_free(&cha_tx);
    mbedtls_ecp_group_free(&ecdh);
    mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy);
}

void Crypto::gen_bytes(std::span<uint8_t> buffer) {
    assert(mbedtls_ctr_drbg_random(&rng, buffer.data(), buffer.size()) == 0);
}

PubKeys Crypto::gen_pubkeys(void) {
    mbedtls_mpi pt_priv;
    mbedtls_ecp_point pt_pub;
    mbedtls_mpi_init(&pt_priv);
    mbedtls_ecp_point_init(&pt_pub);

    PubKeys pair;
    size_t len;
    std::array<uint8_t, 33> buf;
    assert(mbedtls_ecdh_gen_public(&ecdh, &pt_priv, &pt_pub, mbedtls_ctr_drbg_random, &rng) == 0);
    assert(mbedtls_mpi_write_binary(&pt_priv, pair.priv.data(), pair.priv.size()) == 0);
    assert(mbedtls_ecp_point_write_binary(&ecdh, &pt_pub, MBEDTLS_ECP_PF_COMPRESSED, &len, buf.data(), buf.size()) == 0);
    assert(len == buf.size());
    pair.odd = buf[0] == 0x03; // else 0x02
    std::memcpy(pair.pub.data(), &buf[1], pair.pub.size());

    mbedtls_ecp_point_free(&pt_pub);
    mbedtls_mpi_free(&pt_priv);
    return pair;
}

std::array<uint8_t, 32> Crypto::compute_secret(const PubKeys &pair) {
    mbedtls_mpi pt_secret, pt_priv;
    mbedtls_ecp_point pt_pub;
    mbedtls_mpi_init(&pt_secret);
    mbedtls_mpi_init(&pt_priv);
    mbedtls_ecp_point_init(&pt_pub);

    std::array<uint8_t, 32> secret;
    std::array<uint8_t, 33> tmp;
    tmp[0] = pair.odd ? 0x03: 0x02;
    std::memcpy(&tmp[1], pair.pub.data(), pair.pub.size());
    assert(mbedtls_ecp_point_read_binary(&ecdh, &pt_pub, tmp.data(), tmp.size()) == 0);
    assert(mbedtls_mpi_read_binary(&pt_priv, pair.priv.data(), pair.priv.size()) == 0);
    assert(mbedtls_ecdh_compute_shared(&ecdh, &pt_secret, &pt_pub, &pt_priv, mbedtls_ctr_drbg_random, &rng) == 0);
    assert(mbedtls_mpi_write_binary(&pt_secret, secret.data(), secret.size()) == 0);

    mbedtls_ecp_point_free(&pt_pub);
    mbedtls_mpi_free(&pt_priv);
    mbedtls_mpi_free(&pt_secret);
    return secret;
}

SymKeys Crypto::compute_symkeys(std::span<const uint8_t, 32> secret) {
    SymKeys keys;
    assert(mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), nullptr, 0,
        secret.data(), secret.size(), nullptr, 0, reinterpret_cast<uint8_t*>(&keys), sizeof(keys)) == 0);
    return keys;
}

void Crypto::set_keys(std::span<const uint8_t, 32> tx, std::span<const uint8_t, 32> rx) {
    assert(mbedtls_chachapoly_setkey(&cha_tx, tx.data()) == 0);
    assert(mbedtls_chachapoly_setkey(&cha_rx, rx.data()) == 0);
    assert(mbedtls_ctr_drbg_random(&rng, reinterpret_cast<uint8_t*>(&cha_tx_ctr), sizeof(cha_tx_ctr)) == 0);
}

EncryptedPacket Crypto::encrypt(const Payload &data, const EncryptedPacket &prev) {
    // construct plaintext (8-byte response + 8-byte payload)
    std::array<uint8_t, 32> sha256;
    assert(mbedtls_sha256(reinterpret_cast<const uint8_t*>(&prev), sizeof(prev), sha256.data(), 0) == 0);

    std::array<uint8_t, 16> plaintext;
    std::memcpy(&plaintext[0], sha256.data(), 8); // truncate SHA256 to 8 bytes
    std::memcpy(&plaintext[8], data.raw.data(), 8);

    // generate packet
    EncryptedPacket pkt;
    std::array<uint8_t, 16> tag;
    for (auto i = 0; i < pkt.nonce.size(); i++) {
        pkt.nonce[i] = (cha_tx_ctr >> (8 * i)) & 0xFF;
    }
    cha_tx_ctr++;
    assert(mbedtls_chachapoly_encrypt_and_tag(&cha_tx, plaintext.size(), pkt.nonce.data(),
        nullptr, 0, plaintext.data(), pkt.ciphertext.data(), tag.data()) == 0);
    std::memcpy(pkt.tag.data(), tag.data(), pkt.tag.size());
    return pkt;
}

std::optional<Payload> Crypto::decrypt(const EncryptedPacket &pkt, const EncryptedPacket *prev) {
    // adapted from mbedtls_chachapoly_auth_decrypt() for short tag
    std::array<uint8_t, 16> plaintext, tag;
    assert(mbedtls_chachapoly_starts(&cha_rx, pkt.nonce.data(), MBEDTLS_CHACHAPOLY_DECRYPT) == 0);
    assert(mbedtls_chachapoly_update_aad(&cha_rx, nullptr, 0) == 0);
    assert(mbedtls_chachapoly_update(&cha_rx, plaintext.size(), pkt.ciphertext.data(), plaintext.data()) == 0);
    assert(mbedtls_chachapoly_finish(&cha_rx, tag.data()) == 0);
    volatile bool tag_valid = mbedtls_ct_memcmp(tag.data(), pkt.tag.data(), pkt.tag.size()) == 0;

    // always check response too bc of timing attacks
    volatile bool resp_valid = true;
    if (prev) {
        std::array<uint8_t, 32> sha256;
        assert(mbedtls_sha256(reinterpret_cast<const uint8_t*>(prev), sizeof(*prev), sha256.data(), 0) == 0);
        resp_valid = mbedtls_ct_memcmp(sha256.data(), &plaintext[0], 8) == 0;
    }

    // bitwise for constant time
    if (tag_valid & resp_valid) {
        Payload resp;
        std::memcpy(resp.raw.data(), &plaintext[8], 8);
        return resp;
    } else {
        return std::nullopt;
    }
}

Radio::Radio(bool host) : host(host), tx(ctx), rx(ctx) {
    tx.open(asio::ip::udp::v4());
    rx.open(asio::ip::udp::v4());

    asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), host ? 8000 : 8010);
    rx.bind(ep);
    rx.non_blocking(true);
}

void Radio::send(uint8_t chan, uint8_t pipe, std::span<const uint8_t, 32> payload) {
    asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), host ? 8010 : 8000);
    std::array<uint8_t, 34> buffer;
    buffer[0] = chan;
    buffer[1] = pipe;
    std::memcpy(&buffer[2], payload.data(), payload.size());
    std::this_thread::sleep_for(130us);
    tx.send_to(asio::buffer(buffer), ep);
    rx_chan = std::nullopt;
}

std::optional<uint8_t> Radio::recv(uint8_t chan, std::span<uint8_t, 32> buffer) {
    std::array<uint8_t, 34> buf;
    asio::ip::udp::endpoint ep;
    if (!rx_chan.has_value() || rx_chan.value() != chan) {
        std::this_thread::sleep_for(130us);
        while (true) {
            try {
                rx.receive_from(asio::buffer(buf), ep);
            } catch (const std::system_error &e) {
                break;
            }
        }
        rx_chan = chan;
    }
    try {
        auto len = rx.receive_from(asio::buffer(buf), ep);
        if (len != buf.size() || buf[0] != rx_chan.value()) {
            return std::nullopt;
        }
        uint8_t pipe = buf[1];
        std::memcpy(buffer.data(), &buf[2], buffer.size());
        return pipe;
    } catch (const std::system_error &e) {
        return std::nullopt;
    }
}

Protocol::Protocol(Crypto &crypt, Radio &rf) : crypt(crypt), rf(rf) {}

SymKeys Protocol::pair(uint32_t retry_ms) {
    PubKeys tx_keys = crypt.gen_pubkeys(); // new key for each try
    PubKeys rx_keys { .priv = tx_keys.priv };
    while (true) {
        rf.send(0x00, tx_keys.odd ? 0x01 : 0x02, tx_keys.pub);
        auto pipe = poll(0x00, rx_keys.pub, retry_ms);
        if (pipe.has_value() && (pipe.value() == 0x01 || pipe.value() == 0x02)) {
            rx_keys.odd = pipe.value() == 0x01;
            break;
        }
    }
    auto start = std::chrono::high_resolution_clock::now();
    while ((std::chrono::high_resolution_clock::now() - start) < 100ms) {
        rf.send(0x00, tx_keys.odd ? 0x01 : 0x02, tx_keys.pub);
    }
    return crypt.compute_symkeys(crypt.compute_secret(rx_keys));
}

std::optional<uint8_t> Protocol::poll(uint8_t chan, std::span<uint8_t, 32> buffer, uint32_t timeout_ms) {
    auto start = std::chrono::high_resolution_clock::now();
    while ((std::chrono::high_resolution_clock::now() - start) < std::chrono::milliseconds(timeout_ms)) {
        auto pipe = rf.recv(chan, buffer);
        if (pipe.has_value()) {
            return pipe.value();
        }
    }
    return std::nullopt;
}
