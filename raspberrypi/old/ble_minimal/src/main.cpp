#include <pico/stdlib.h>
#include <tusb.h>
#include <hci_dump.h>
#include <hci_dump_embedded_stdout.h>
#include "ble.h"

static void cpu0_thread(void) {
    bool send_key = false;
    absolute_time_t next = get_absolute_time();
    BLE ble;

    while (true) {
        hid_keyboard_report_t kb_report = {0};
        hid_mouse_report_t mouse_report = {0};
        uint16_t consumer_report = 0;

        if (send_key) {
            kb_report.keycode[0] = HID_KEY_CAPS_LOCK;
        }
        if (time_reached(next)) {
            // printf("conn: %d tick: %d leds: 0x%02x\r\n", ble.connected(), send_key, ble.led_status());
            send_key = !send_key;
            next = make_timeout_time_ms(500);
        }

        ble.process();
        ble.set_report(kb_report, mouse_report, consumer_report);
    }
}

void dummy_log_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len) {
    (void) packet_type;
    (void) in;
    (void) packet;
    (void) len;
}

int main() {
    stdio_init_all();

    // default dump logs all packets
    hci_dump_t dump = *hci_dump_embedded_stdout_get_instance();
    dump.log_packet = dummy_log_packet;
    hci_dump_init(&dump);

    while (!tud_cdc_connected());

    // TODO re-enable multicore once BTstack port more stable
    cpu0_thread();
}
