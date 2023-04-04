#include <tusb.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include "ble.h"

BLE ble;

void thread0(void) {
    printf("thread0 booted!\r\n");

    while (true) {
        ble.process();
        sleep_us(1000); // appears to work well enough
    }
}

void thread1(void) {
    printf("thread1 booted!\r\n");

    int8_t i = 0;
    hid_keyboard_report_t kb = {0};
    hid_mouse_report_t mouse = {0};
    uint16_t consumer = 0;
    while (true) {
        printf("thread1 pressed A: %d\r\n", i++);
        kb.keycode[0] = HID_KEY_CAPS_LOCK;
        ble.set_report(kb, mouse, consumer);
        sleep_ms(50);
        kb.keycode[0] = HID_KEY_NONE;
        ble.set_report(kb, mouse, consumer);
        sleep_ms(1000);
    }
}

int main(void) {
    stdio_init_all();
    while (!tud_cdc_connected());

    multicore_launch_core1(thread0); // cpu1
    thread1(); // cpu0
}
