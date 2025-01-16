#pragma once

#include <optional>

namespace tinyusb {
#include <class/hid/hid.h>
#include <class/hid/hid_device.h>
};

using tinyusb::hid_keyboard_report_t;
using tinyusb::hid_mouse_report_t;

class BLE {
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

    bool connected(void);
    std::optional<uint32_t> passkey(void);

    void set_batt(uint8_t level);

    bool idle(void);
    void send(hid_keyboard_report_t &report);
    void send(hid_mouse_report_t &report);

    // TODO caps lock?
};
