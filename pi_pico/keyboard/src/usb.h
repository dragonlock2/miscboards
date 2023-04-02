#ifndef USB_H
#define USB_H

#include <tusb.h>
#include <hardware/sync.h>

class USB {
public:
    USB(void);

    void process(void);
    void set_report(hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer);
    bool connected(void);

private:
    spin_lock_t* lock;
    bool hid_started;
};

#endif // USB_H
