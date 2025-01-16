#include <btstack.h>
#include <pico/stdlib.h>
#include "ble_minimal_gatt_header/hid.h"
#include "ble.h"

static struct {
    BLE *inst;
    bool connected;
    std::optional<uint32_t> passkey;
    BLE::passkey_get_cb passkey_get;
} data;

static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            data.connected = false;
            break;
    }
}

static void sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            data.passkey = sm_event_passkey_display_number_get_passkey(packet);
            break;

        case SM_EVENT_PASSKEY_INPUT_NUMBER:
            if (data.passkey_get) {
                auto pkey = data.passkey_get();
                sm_passkey_input(sm_event_passkey_input_number_get_handle(packet), pkey);
            }
            break;

        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            data.passkey = sm_event_numeric_comparison_request_get_passkey(packet);
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            break;

        case SM_EVENT_PAIRING_COMPLETE:
            data.passkey = std::nullopt;
            if (sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS) {
                data.connected = true;
            }
            break;

        case SM_EVENT_REENCRYPTION_COMPLETE:
            switch (sm_event_reencryption_complete_get_status(packet)) {
                case ERROR_CODE_SUCCESS:
                    data.connected = true;
                    break;

                case ERROR_CODE_PIN_OR_KEY_MISSING: {
                    bd_addr_t addr;
                    sm_event_reencryption_complete_get_address(packet, addr);
                    bd_addr_type_t addr_type = static_cast<bd_addr_type_t>(
                        sm_event_reencryption_started_get_addr_type(packet));
                    gap_delete_bonding(addr_type, addr);
                    sm_request_pairing(sm_event_reencryption_complete_get_handle(packet));
                    break;
                }
            }
            break;
    }
}

static void hids_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    // TODO implement
    auto type = hci_event_packet_get_type(packet);
    printf("hids %d\r\n", type);
}

BLE::BLE(conn_type ct, passkey_get_cb cb) {
    configASSERT(data.inst == nullptr);
    data.inst = this;
    data.passkey_get = cb;

    l2cap_init();
    sm_init();
    switch (ct) {
        case conn_type::JUST_WORKS:      sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT); break;
        case conn_type::PASSKEY_DISPLAY: sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);       break;
        case conn_type::PASSKEY_INPUT:   sm_set_io_capabilities(IO_CAPABILITY_KEYBOARD_ONLY);      break;
        case conn_type::PASSKEY_CONFIRM: sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_YES_NO);     break;
    }
    sm_set_secure_connections_only_mode(true);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING);
    att_server_init(profile_data, nullptr, nullptr);

    using namespace tinyusb;
    static const uint8_t HID_DESC[] = {
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)), // matches hid.gatt
    };
    battery_service_server_init(100);
    device_information_service_server_init();
    hids_device_init(0, HID_DESC, sizeof(HID_DESC));

    static const uint8_t ADV_DATA[] = {
        0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
        0x09, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'k', 'e', 'y', 'b', 'o', 'a', 'r', 'd',
        0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
            ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xFF, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
        0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
    };
    uint16_t  adv_int_min = 0x0030;
    uint16_t  adv_int_max = 0x0030;
    uint8_t   adv_type    = 0;
    bd_addr_t null_addr   = {0};
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(ADV_DATA), const_cast<uint8_t*>(ADV_DATA));
    gap_advertisements_enable(1);

    static btstack_packet_callback_registration_t hci_cb_reg;
    static btstack_packet_callback_registration_t sm_cb_reg;
    hci_cb_reg.callback = hci_packet_handler;
    sm_cb_reg.callback  = sm_packet_handler;
    hci_add_event_handler(&hci_cb_reg);
    sm_add_event_handler(&sm_cb_reg);
    hids_device_register_packet_handler(hids_packet_handler);

    hci_power_control(HCI_POWER_ON);
}

bool BLE::connected(void) {
    return data.connected;
}

std::optional<uint32_t> BLE::passkey(void) {
    return data.passkey;
}

void BLE::set_batt(uint8_t level) {
    battery_service_server_set_battery_value(level);
}
