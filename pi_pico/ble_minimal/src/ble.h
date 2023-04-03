#ifndef BLE_H
#define BLE_H

#include <class/hid/hid.h>
#include <class/hid/hid_device.h>
#include <pico/stdlib.h>
#include <hardware/sync.h>

class BLE {
public:
    BLE(void);

    void process(void);
    void set_report(hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer);

private:
    spin_lock_t* lock;
    bool started;
};

#endif // BLE_H
