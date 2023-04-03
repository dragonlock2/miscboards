#ifndef BLE_H
#define BLE_H

#include <hardware/sync.h>
#include "config.h"

class BLE {
public:
    BLE(void);

    void process(void);
    void set_report(hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer);
    bool connected(void);
    uint8_t led_status(void);

private:
    spin_lock_t* lock;
};

#endif // BLE_H
