#pragma once

#include <optional>
#include "hid.h"

class USB : public HID {
public:
    USB(void);

    USB(USB&) = delete;
    USB(USB&&) = delete;

    bool connected(void) override;

    uint8_t led(void) override; // fails on macOS

    bool send(keyboard_report_t &report) override;
    bool send(mouse_report_t &report) override;
    bool send(consumer_report_t &report) override;
};
