#pragma once

#include <optional>
#include "hid.h"

class BLE : public HID {
public:
    enum class conn_type {
        JUST_WORKS,
        PASSKEY_DISPLAY, // fails on Linux/macOS
        PASSKEY_INPUT,
        PASSKEY_CONFIRM,
    };
    typedef uint32_t (*passkey_get_cb)(void);

    BLE(conn_type ct, passkey_get_cb cb=nullptr);

    BLE(BLE&) = delete;
    BLE(BLE&&) = delete;

    bool connected(void) override;
    std::optional<uint32_t> passkey(void);

    uint8_t led(void) override; // fails on macOS
    void set_batt(uint8_t level);

    bool send(keyboard_report_t &report) override;
    bool send(mouse_report_t &report) override;
    bool send(consumer_report_t &report) override;
};
