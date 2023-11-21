#include <tusb.h>
#include "usb.h"

/* private defines */

#if (1) // TODO remove

#include <cstring>

typedef uint32_t uint;

#define USB_VID (0x0069)
#define USB_PID (0x0421) // 0x0420 is used by JABI

#define EPNUM_HID (0x81)
#define POLL_RATE (10) // ms

enum class usb_string_desc {
    LANGID        = 0x00,
    MANUFACTURER  = 0x01,
    PRODUCT       = 0x02,
    SERIAL_NUMBER = 0x03,
    COUNT
};

enum class hid_report_id {
    KEYBOARD = 1,
    MOUSE = 2,
    COUNT
};

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110, // USB 1.1 (not high speed)
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(usb_string_desc::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(usb_string_desc::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(usb_string_desc::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

static const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::KEYBOARD))),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::MOUSE   ))),
};

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN), TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500), // 500mA
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, POLL_RATE)
};

static char const* desc_strings[] = { // keep <= 127 chars due to encoding
    [static_cast<uint>(usb_string_desc::LANGID)]        = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
    [static_cast<uint>(usb_string_desc::MANUFACTURER)]  = "Matt Tran",
    [static_cast<uint>(usb_string_desc::PRODUCT)]       = "Keyboard",
    [static_cast<uint>(usb_string_desc::SERIAL_NUMBER)] = "69420",
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
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
        for (uint i = 0; i < strlen(s); i++) {
            str[1 + i] = s[i]; // ASCII to UTF-16
        }
        count = strlen(s);
    }
    str[0] = (TUSB_DESC_STRING << 8) | (2 + 2 * count);
    return str;
}

static hid_keyboard_report_t kb_report;
static hid_mouse_report_t mouse_report;

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
    // tud_hid_report(static_cast<uint8_t>(hid_report_id::KEYBOARD), &kb_report, sizeof(kb_report));
    // tud_hid_report(static_cast<uint8_t>(hid_report_id::MOUSE), &mouse_report, sizeof(mouse_report));
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
    if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == static_cast<uint8_t>(hid_report_id::KEYBOARD) && bufsize > 0) {
        printf("kb status 0x%0x\r\n", buffer[0]);
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

#endif // TODO remove

/* private data */

/* private helpers */

/* public functions */
void usb_task(void *args) {
    (void) args;
    tusb_init();

    while (1) {
        // TODO remove
        mouse_report.x = 10;
        mouse_report.y = 10;

        static bool hid_started = false;
        if (!hid_started && tud_mounted() && !tud_suspended() && tud_hid_ready()) {
            // most keyboard/mice only send when there's a change, should do that
            tud_hid_report(static_cast<uint8_t>(hid_report_id::KEYBOARD), &kb_report, sizeof(kb_report));
            tud_hid_report(static_cast<uint8_t>(hid_report_id::MOUSE), &mouse_report, sizeof(mouse_report));
            hid_started = true;
        } else {
            hid_started = false;
        }

        tud_task();
    }

    vTaskDelete(NULL);
}
