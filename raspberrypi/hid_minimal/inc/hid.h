#pragma once

class HID {
public:
    // report definitions match TinyUSB
    struct keyboard_report_t {
        uint8_t modifier;
        uint8_t reserved;
        uint8_t keycode[6];
    };

    struct mouse_report_t {
        uint8_t buttons;
        int8_t  x;
        int8_t  y;
        int8_t  wheel;
        int8_t  pan;
    };

    typedef uint16_t consumer_report_t;

    // API
    virtual bool connected(void) = 0;
    virtual uint8_t led(void) = 0;
    virtual bool send(keyboard_report_t &report) = 0;
    virtual bool send(mouse_report_t &report) = 0;
    virtual bool send(consumer_report_t &report) = 0;
};
