#include <cstring>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <tusb.h>
#include <pico/stdlib.h>
#include "usb.h"

/* private defines */
static constexpr uint8_t EP_HID = 0x81;

enum class string_id {
    LANGID,
    MANUFACTURER,
    PRODUCT,
    SERIAL_NUMBER,
    COUNT,
};

enum class report_id {
    KEYBOARD = 1,
    MOUSE    = 2,
    CONSUMER = 3,
};

static const tusb_desc_device_t DEVICE_DESCRIPTOR = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xcafe,
    .idProduct          = 0xbeef,
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(string_id::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(string_id::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(string_id::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

static const uint8_t HID_DESCRIPTOR[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(report_id::KEYBOARD))),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(report_id::MOUSE   ))),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(static_cast<uint8_t>(report_id::CONSUMER))),
};

static const uint8_t CONFIG_DESCRIPTOR[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN), TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(HID_DESCRIPTOR), EP_HID, CFG_TUD_HID_EP_BUFSIZE, 1)
};

static char const *STRING_DESCRIPTOR[] = {
    [static_cast<uint>(string_id::LANGID)]        = (const char[]) { 0x09, 0x04 }, // English (0x0409)
    [static_cast<uint>(string_id::MANUFACTURER)]  = "miscboards",
    [static_cast<uint>(string_id::PRODUCT)]       = "keyboard",
    [static_cast<uint>(string_id::SERIAL_NUMBER)] = "hid_minimal",
};

/* private data */
static struct {
    USB *inst;
    SemaphoreHandle_t lock, rep_lock;
    uint8_t led;
    TaskHandle_t waiter;
} data;

/* private helpers */
template<typename T>
static bool report_send(T& report, uint8_t id) {
    bool ret = false;
    xSemaphoreTake(data.rep_lock, portMAX_DELAY);

    // attach waiter
    xSemaphoreTake(data.lock, portMAX_DELAY);
    data.waiter = xTaskGetCurrentTaskHandle();
    xSemaphoreGive(data.lock);

    // wait send
    ulTaskNotifyTakeIndexed(configNOTIF_USB, true, 0); // clear notif in case
    tud_hid_report(id, &report, sizeof(report));
    if (ulTaskNotifyTakeIndexed(configNOTIF_USB, true, pdMS_TO_TICKS(1000)) != 0) {
        ret = true;
    } else {
        xSemaphoreTake(data.lock, portMAX_DELAY);
        data.waiter = nullptr;
        xSemaphoreGive(data.lock);
    }

    xSemaphoreGive(data.rep_lock);
    return ret;
}

static void usb_task(void*) {
    while (true) {
        tud_task();
    }
    vTaskDelete(nullptr);
}

/* public functions */
USB::USB(void) {
    configASSERT(data.inst == nullptr);
    data.inst = this;
    data.lock = xSemaphoreCreateMutex();
    data.rep_lock = xSemaphoreCreateMutex();
    data.led = 0;
    configASSERT(data.lock && data.rep_lock);

    tusb_init();
    configASSERT(xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE,
        NULL, configMAX_PRIORITIES - 1, NULL) == pdPASS);
}

bool USB::connected(void) {
    return tud_hid_ready();
}

uint8_t USB::led(void) {
    return data.led;
}

bool USB::send(keyboard_report_t &report) {
    return report_send(report, static_cast<uint8_t>(report_id::KEYBOARD));
}

bool USB::send(mouse_report_t &report) {
    return report_send(report, static_cast<uint8_t>(report_id::MOUSE));
}

bool USB::send(consumer_report_t &report) {
    return report_send(report, static_cast<uint8_t>(report_id::CONSUMER));
}

uint8_t const *tud_descriptor_device_cb(void) {
    return reinterpret_cast<uint8_t const*>(&DEVICE_DESCRIPTOR);
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return CONFIG_DESCRIPTOR;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return HID_DESCRIPTOR;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t str[127]; // UTF-16
    (void) langid;

    // assumes little-endian!
    uint8_t count = 0;
    if (index >= static_cast<uint8_t>(string_id::COUNT)) {
        return nullptr;
    } else if (index == static_cast<uint8_t>(string_id::LANGID)) {
        std::memcpy(&str[1], STRING_DESCRIPTOR[index], strlen(STRING_DESCRIPTOR[index]));
        count = 1;
    } else {
        const char* s = STRING_DESCRIPTOR[index];
        for (size_t i = 0; i < strlen(s); i++) {
            str[1 + i] = s[i]; // ASCII to UTF-16
        }
        count = strlen(s);
    }
    str[0] = (TUSB_DESC_STRING << 8) | (2 + 2 * count);
    return str;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
    uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
    uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    if ((report_id == static_cast<uint8_t>(report_id::KEYBOARD)) &&
        (report_type == HID_REPORT_TYPE_OUTPUT) &&
        (bufsize == 1)) {
        xSemaphoreTake(data.lock, portMAX_DELAY);
        data.led = buffer[0];
        xSemaphoreGive(data.lock);
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
    xSemaphoreTake(data.lock, portMAX_DELAY);
    if (data.waiter) {
        xTaskNotifyGiveIndexed(data.waiter, configNOTIF_USB);
        data.waiter = nullptr;
    }
    xSemaphoreGive(data.lock);
}
