#include <stdlib.h>
#include <pico/cyw43_arch.h>
#include "ble.h"

namespace bt { // conflicts w/ TinyUSB
#include <btstack.h>
#include "keyboard.h"
};
using namespace bt;

/* BTstack specific (HID over GATT Profile) */
enum class hid_report_id {
    KEYBOARD = 1,
    // MOUSE    = 2,
    // CONSUMER = 3,
    COUNT
};

static const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::KEYBOARD))),
    // TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::MOUSE   ))),
    // TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::CONSUMER))),
};

static const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x0d, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'M', 'a', 't', 't', '\'', 's', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};

static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;

static hid_keyboard_report_t kb_report;
static hid_mouse_report_t mouse_report;
static uint16_t consumer_report;
static uint8_t kb_status;

static void btstack_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    // TODO kb, mouse, consumer, leds
    (void) protocol_mode;
}

/* BLE HID abstractions */
BLE::BLE(void) {
    lock = spin_lock_init(spin_lock_claim_unused(true));
    cyw43_arch_init();

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
    att_server_init(profile_data, NULL, NULL);

    battery_service_server_init(42); // no battery monitor available (yet)
    device_information_service_server_init();
    hids_device_init(0, desc_hid_report, sizeof(desc_hid_report));

    // TODO figure out what these do
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr = {0};
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), const_cast<uint8_t*>(adv_data));
    gap_advertisements_enable(1);

    static btstack_packet_callback_registration_t hci_event_callback_registration = {0};
    hci_event_callback_registration.callback = &btstack_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    static btstack_packet_callback_registration_t sm_event_callback_registration = {0};
    sm_event_callback_registration.callback = &btstack_packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    hids_device_register_packet_handler(btstack_packet_handler);

    // TODO move
    // hci_power_control(HCI_POWER_ON);
    // btstack_run_loop_execute();
}

void BLE::process(void) {
    // async_context_poll(cyw43_arch_async_context()); // TODO works?
}

void BLE::set_report(hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer) {
    uint32_t s = spin_lock_blocking(lock);
    kb_report = kb;
    mouse_report = mouse;
    consumer_report = consumer;
    spin_unlock(lock, s);
}

bool BLE::connected(void) {
    return con_handle != HCI_CON_HANDLE_INVALID;
}

uint8_t BLE::led_status(void) {
    return kb_status;
}
