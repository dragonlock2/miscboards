#ifndef GUI_H
#define GUI_H

#include <cstdarg>
#include "kscan.h"
#include "ws2812b.h"
#include "ssd1306.h"

class GUI {
public:
    GUI(kscan& keys, ssd1306& oled, ws2812b& leds, uint sleep);

    void process(void);
    void display(void);

private:
    void oled_printf(const char* format, ...);

    kscan& keys;
    ssd1306& oled;
    ws2812b& leds;
    const uint sleep;
    uint sleep_ctr;
};

#endif // GUI_H
