#include <stdlib.h>
#include <pico/cyw43_arch.h>
#include "ble.h"

namespace bt { // conflicts w/ TinyUSB
#include <btstack.h>
#include "keyboard.h"
};
using namespace bt;

/* BTstack specific */
enum class hid_report_id {
    KEYBOARD = 1, // matches keyboard.gatt!
    MOUSE    = 2,
    CONSUMER = 3,
    COUNT
};

static const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::KEYBOARD))),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::MOUSE   ))),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::CONSUMER))),
};

static const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x10, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'M', 'a', 't', 't', '\'', 's', ' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};

static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;
static uint32_t con_passkey = 0;
static bool con_passkey_valid = false;

static hid_report_id current_id = hid_report_id::KEYBOARD;

static hid_keyboard_report_t kb_report;
static hid_mouse_report_t mouse_report;
static uint16_t consumer_report;
static uint8_t kb_status;

static bool consumer_txd;

static void send_report(void) {
    switch (current_id) {
        case hid_report_id::KEYBOARD:
            att_server_notify(con_handle, protocol_mode == 0 ?
                    ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_VALUE_HANDLE :
                    ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_VALUE_HANDLE,
                reinterpret_cast<uint8_t*>(&kb_report), sizeof(kb_report));
            current_id = hid_report_id::MOUSE;
            break;

        case hid_report_id::MOUSE:
            att_server_notify(con_handle, protocol_mode == 0 ?
                    ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_VALUE_HANDLE :
                    ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_04_VALUE_HANDLE,
                reinterpret_cast<uint8_t*>(&mouse_report), sizeof(mouse_report));
            current_id = hid_report_id::CONSUMER;
            break;

        case hid_report_id::CONSUMER:
            att_server_notify(con_handle, ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_05_VALUE_HANDLE,
                reinterpret_cast<uint8_t*>(&consumer_report), sizeof(consumer_report));
            consumer_txd = true;
            current_id = hid_report_id::KEYBOARD;
            break;

        default:
            break;
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    if (packet_type != HCI_EVENT_PACKET) { return; }
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            break;

        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            con_passkey = sm_event_numeric_comparison_request_get_passkey(packet);
            con_passkey_valid = true;
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            printf("numeric comparison: %ld\r\n", con_passkey);
            break;

        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            con_passkey = sm_event_passkey_display_number_get_passkey(packet);
            con_passkey_valid = true;
            printf("passkey: %ld\r\n", con_passkey);
            break;

        case SM_EVENT_PAIRING_COMPLETE:
            con_passkey_valid = false;
            printf("pairing complete\r\n");
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)){
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    break;

                case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                    break;

                case HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_boot_mouse_input_report_enable_get_con_handle(packet);
                    break;

                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                    break;

                case HIDS_SUBEVENT_CAN_SEND_NOW:
                    send_report();
                    break;

                default:
                    break;
            }
            break;
            
        default:
            break;
    }
}

/* BLE abstractions */
BLE::BLE(void) {
    printf("Starting BLE...\r\n");
    lock = spin_lock_init(spin_lock_claim_unused(true));
    cyw43_arch_init();

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
    sm_set_secure_connections_only_mode(true);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING);
    att_server_init(profile_data, NULL, NULL);

    battery_service_server_init(100); // no battery monitor yet
    device_information_service_server_init();
    hids_device_init(0, desc_hid_report, sizeof(desc_hid_report));

    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr = {0};
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), const_cast<uint8_t*>(adv_data));
    gap_advertisements_enable(1);

    static btstack_packet_callback_registration_t hci_event_callback_registration = {0};
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    static btstack_packet_callback_registration_t sm_event_callback_registration = {0};
    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    hids_device_register_packet_handler(packet_handler);

    hci_power_control(HCI_POWER_ON);
    printf("Started BLE!\r\n");
}

void BLE::process(void) {
    if (con_handle != HCI_CON_HANDLE_INVALID && att_server_can_send_packet_now(con_handle)) {
        hids_device_request_can_send_now_event(con_handle);
    }

    // async_context_poll(cyw43_arch_async_context()); // unnecessary w/ threadsafe_background
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

bool BLE::consumer_transmitted(void) {
    // note doesn't guarantee last set report transmitted
    bool tx = consumer_txd;
    if (tx) {
        uint32_t s = spin_lock_blocking(lock);
        consumer_txd = false;
        spin_unlock(lock, s);
    }
    return tx;
}

uint8_t BLE::led_status(void) {
    return kb_status; // TODO implement
}

bool BLE::passkey(uint32_t& key) {
    key = con_passkey;
    return con_passkey_valid;
}
