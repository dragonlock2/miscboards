#ifndef BLE_H
#define BLE_H

#include <class/hid/hid.h>
#include <class/hid/hid_device.h>
#include <pico/stdlib.h>

class BLE {
public:
    BLE(void);

    void process(void);

private:
    bool started;
};

#endif // BLE_H
