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
    void set_report(hid_keyboard_report_t& kb);

private:
    spin_lock_t* lock;
    bool started;
};

#endif // BLE_H
