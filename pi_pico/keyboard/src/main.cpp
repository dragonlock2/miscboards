#include <pico/stdlib.h>
#include <tusb.h>
#include "config.h"
#include "kscan.h"
#include "encoder.h"
#include "usb.h"
#include "macro.h"
#include "gui.h"

static kscan   keys(board_cfg);
static encoder enc(board_cfg);
static USB     usb;
static BLE     ble;
static macro   mac(keys, board_cfg, board_map);

static ssd1306 oled(board_cfg);
static ws2812b leds(board_cfg);
static GUI     gui(keys, usb, ble, oled, leds, board_cfg.sleep);

static void cpu0_thread(void) {
    uint64_t cpu0_time = 0;
    int ticks = 0;

    while (true) {
        uint64_t s = time_us_64();

        keys.scan();

        int diff = enc;
        ticks += diff;

        gui.process(diff, cpu0_time);
        gui.display();

        hid_keyboard_report_t kb_report = {0};
        hid_mouse_report_t mouse_report = {0};
        uint16_t consumer_report = 0;
        bool consumer_txd = usb.consumer_transmitted() || ble.consumer_transmitted();
        mac.get_report(ticks, consumer_txd, kb_report, mouse_report, consumer_report);

        usb.process();
        usb.set_report(kb_report, mouse_report, consumer_report);

        ble.process();
        ble.set_report(kb_report, mouse_report, consumer_report);

        cpu0_time = time_us_64() - s;
    }
}

int main() {
    // TODO re-enable multicore once BTstack port more stable
    cpu0_thread();
}
