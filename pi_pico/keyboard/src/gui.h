#ifndef GUI_H
#define GUI_H

#include <cstdarg>
#include "kscan.h"
#include "usb.h"
#include "ble.h"
#include "ws2812b.h"
#include "ssd1306.h"

class GUI {
public:
    GUI(kscan& keys, USB& usb, BLE& ble, ssd1306& oled, ws2812b& leds, uint sleep);

    void process(void);
    void display(void);

private:
    void oled_printf(const char* format, ...);
    bool sleep_check(int enc);

    kscan& keys;
    USB& usb;
    BLE& ble;
    ssd1306& oled;
    ws2812b& leds;
    const uint sleep;
    uint sleep_ctr;

    // GUI state
    uint frame_ctr;
    int enc_ticks;
};

#endif // GUI_H
