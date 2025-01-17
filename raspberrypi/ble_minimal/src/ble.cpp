#include <array>
#include <variant>
#include <btstack.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include "ble_minimal_gatt_header/hid.h"
#include "ble.h"

enum class report_id {
    KEYBOARD = 1, // matches hid.gatt
    MOUSE    = 4,
    CONSUMER = 5,
};

static struct {
    BLE *inst;
    SemaphoreHandle_t lock, rep_lock;
    bool connected;
    std::optional<uint32_t> passkey;
    BLE::passkey_get_cb passkey_get;
    hci_con_handle_t handle;
    bool boot_mode;
    TaskHandle_t waiter;
    std::variant<hid_keyboard_report_t, hid_mouse_report_t,
        hid_consumer_report_t> report;
} data;

template<typename T>
bool report_send(T& report) {
    bool ret = false;
    xSemaphoreTake(data.rep_lock, portMAX_DELAY);

    // attach waiter
    xSemaphoreTake(data.lock, portMAX_DELAY);
    bool send = data.handle != HCI_CON_HANDLE_INVALID;
    if (send) {
        data.waiter = xTaskGetCurrentTaskHandle();
        data.report = report;
    }
    xSemaphoreGive(data.lock);

    // wait send
    if (send) {
        ulTaskNotifyTakeIndexed(configNOTIF_BLE, true, 0); // clear notif in case
        hids_device_request_can_send_now_event(data.handle);
        if (ulTaskNotifyTakeIndexed(configNOTIF_BLE, true, pdMS_TO_TICKS(1000)) != 0) {
            ret = true;
        } else {
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.waiter = nullptr;
            xSemaphoreGive(data.lock);
        }
    }

    xSemaphoreGive(data.rep_lock);
    return ret;
}

static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.connected = false;
            data.handle = HCI_CON_HANDLE_INVALID;
            xSemaphoreGive(data.lock);
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
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.passkey = sm_event_passkey_display_number_get_passkey(packet);
            xSemaphoreGive(data.lock);
            break;

        case SM_EVENT_PASSKEY_INPUT_NUMBER:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            if (data.passkey_get) {
                auto pkey = data.passkey_get();
                sm_passkey_input(sm_event_passkey_input_number_get_handle(packet), pkey);
            }
            xSemaphoreGive(data.lock);
            break;

        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.passkey = sm_event_numeric_comparison_request_get_passkey(packet);
            xSemaphoreGive(data.lock);
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            break;

        case SM_EVENT_PAIRING_COMPLETE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.passkey = std::nullopt;
            if (sm_event_pairing_complete_get_status(packet) == ERROR_CODE_SUCCESS) {
                data.connected = true;
                data.handle = HCI_CON_HANDLE_INVALID;
                data.boot_mode = false;
            }
            xSemaphoreGive(data.lock);
            break;

        case SM_EVENT_REENCRYPTION_COMPLETE:
            switch (sm_event_reencryption_complete_get_status(packet)) {
                case ERROR_CODE_SUCCESS:
                    xSemaphoreTake(data.lock, portMAX_DELAY);
                    data.connected = true;
                    data.handle = HCI_CON_HANDLE_INVALID;
                    data.boot_mode = false;
                    xSemaphoreGive(data.lock);
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
    if ((packet_type != HCI_EVENT_PACKET) ||
        (hci_event_packet_get_type(packet) != HCI_EVENT_HIDS_META)) {
        return;
    }
    switch (hci_event_hids_meta_get_subevent_code(packet)) {
        case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.handle = hids_subevent_input_report_enable_get_con_handle(packet);
            xSemaphoreGive(data.lock);
            break;

        case HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.handle = hids_subevent_boot_mouse_input_report_enable_get_con_handle(packet);
            xSemaphoreGive(data.lock);
            break;

        case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
            xSemaphoreGive(data.lock);
            break;

        case HIDS_SUBEVENT_PROTOCOL_MODE:
            xSemaphoreTake(data.lock, portMAX_DELAY);
            data.boot_mode = hids_subevent_protocol_mode_get_protocol_mode(packet) == 0;
            xSemaphoreGive(data.lock);
            break;

        case HIDS_SUBEVENT_CAN_SEND_NOW: {
            // BTstack functions may recursively call handler => no lock holding => copy fields
            xSemaphoreTake(data.lock, portMAX_DELAY);
            auto handle = data.handle;
            auto boot_mode = data.boot_mode;
            auto report = data.report;
            bool send = false;
            if (data.waiter) {
                send = true;
                xTaskNotifyGiveIndexed(data.waiter, configNOTIF_BLE);
                data.waiter = nullptr;
            }
            xSemaphoreGive(data.lock);

            // send packet now
            if (send) {
                if (std::holds_alternative<hid_keyboard_report_t>(report)) {
                    auto &rep = std::get<hid_keyboard_report_t>(report);
                    if (boot_mode) {
                        hids_device_send_boot_keyboard_input_report(handle,
                            reinterpret_cast<uint8_t*>(&rep), sizeof(rep));
                    } else {
                        hids_device_send_input_report_for_id(handle, static_cast<uint16_t>(report_id::KEYBOARD),
                            reinterpret_cast<uint8_t*>(&rep), sizeof(rep));
                    }
                } else if (std::holds_alternative<hid_mouse_report_t>(report)) {
                    auto &rep = std::get<hid_mouse_report_t>(report);
                    if (boot_mode) {
                        hids_device_send_boot_mouse_input_report(handle,
                            reinterpret_cast<uint8_t*>(&rep), sizeof(rep));
                    } else {
                        hids_device_send_input_report_for_id(handle, static_cast<uint16_t>(report_id::MOUSE),
                            reinterpret_cast<uint8_t*>(&rep), sizeof(rep));
                    }
                } else if (std::holds_alternative<hid_consumer_report_t>(report)) {
                    auto &rep = std::get<hid_consumer_report_t>(report);
                    hids_device_send_input_report_for_id(handle, static_cast<uint16_t>(report_id::CONSUMER),
                        reinterpret_cast<uint8_t*>(&rep), sizeof(rep));
                }
            }
            break;
        }
    }
}

BLE::BLE(conn_type ct, passkey_get_cb cb) {
    configASSERT(data.inst == nullptr);
    data.inst = this;
    data.lock = xSemaphoreCreateMutex();
    data.rep_lock = xSemaphoreCreateMutex();
    data.passkey_get = cb;
    data.handle = HCI_CON_HANDLE_INVALID;
    configASSERT(data.lock && data.rep_lock);

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
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(report_id::KEYBOARD))),
        TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(report_id::MOUSE))),
        TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(static_cast<uint8_t>(report_id::CONSUMER))),
    };
    static std::array<hids_device_report_t, 5> reports; // 3 input, 1 output, 1 feature
    battery_service_server_init(100);
    device_information_service_server_init();
    hids_device_init_with_storage(0, HID_DESC, sizeof(HID_DESC), reports.size(), reports.data());

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

bool BLE::send(hid_keyboard_report_t &report) {
    return report_send(report);
}

bool BLE::send(hid_mouse_report_t &report) {
    return report_send(report);
}

bool BLE::send(hid_consumer_report_t &report) {
    return report_send(report);
}
