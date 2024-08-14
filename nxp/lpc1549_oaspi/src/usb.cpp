#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include "usb.h"

/* private defines */
enum class usb_string_desc {
    LANGID,
    MANUFACTURER,
    PRODUCT,
    SERIAL_NUMBER,
    COUNT
};

enum class hid_report_id {
    MOUSE = 1,
};

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xcafe,
    .idProduct          = 0x0069,
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(usb_string_desc::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(usb_string_desc::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(usb_string_desc::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

static const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::MOUSE))),
};

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN), TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500), // 500mA
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), 0x81, CFG_TUD_HID_EP_BUFSIZE, 1)
};

static char const* desc_strings[] = { // keep <= 127 chars due to encoding
    [static_cast<size_t>(usb_string_desc::LANGID)]        = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
    [static_cast<size_t>(usb_string_desc::MANUFACTURER)]  = "miscboards",
    [static_cast<size_t>(usb_string_desc::PRODUCT)]       = "lpc1549_oaspi",
    [static_cast<size_t>(usb_string_desc::SERIAL_NUMBER)] = "69420",
};

/* private helpers */
static void usb_handler(void) {
    tud_int_handler(0);
}

static void usb_task(void*) {
    while (true) {
        tud_task();
    }
    vTaskDelete(NULL);
}

/* public functions */
void usb_init(void) {
    NVIC_SetVector(USB0_IRQn, reinterpret_cast<uint32_t>(usb_handler));
    NVIC_SetPriority(USB0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));

    Chip_USB_Init();

    tusb_init();
    xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}

uint8_t const* tud_descriptor_device_cb(void) {
    return reinterpret_cast<uint8_t const*>(&desc_device);
}

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_hid_report;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // only 1 config descriptor
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t str[127]; // UTF-16
    (void) langid;

    // assumes little-endian!
    uint8_t count;
    if (index >= static_cast<uint8_t>(usb_string_desc::COUNT)) {
        return NULL;
    } else if (index == static_cast<uint8_t>(usb_string_desc::LANGID)) {
        memcpy(&str[1], desc_strings[index], strlen(desc_strings[index]));
        count = 1;
    } else {
        const char* s = desc_strings[index];
        for (size_t i = 0; i < strlen(s); i++) {
            str[1 + i] = s[i]; // ASCII to UTF-16
        }
        count = strlen(s);
    }
    str[0] = (TUSB_DESC_STRING << 8) | (2 + 2 * count);
    return str;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}
