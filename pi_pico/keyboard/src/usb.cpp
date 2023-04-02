#include "usb.h"

/* TinyUSB specific */
#define USB_VID (0x0069)
#define USB_PID (0x0421) // 0x0420 is used by JABI

#define EPNUM_HID (0x81)
#define POLL_RATE (1) // ms

enum class usb_string_desc {
    LANGID        = 0x00,
    MANUFACTURER  = 0x01,
    PRODUCT       = 0x02,
    SERIAL_NUMBER = 0x03,
    COUNT
};

enum class hid_report_id {
    KEYBOARD = 1,
    MOUSE    = 2,
    CONSUMER = 3,
    COUNT
};

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // USB 2.0
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

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::KEYBOARD))),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::MOUSE   ))),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(static_cast<uint8_t>(hid_report_id::CONSUMER))),
};

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN), TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500), // 500mA
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, POLL_RATE)
};

char const* desc_strings[] = { // keep <= 127 chars due to encoding
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
static uint16_t consumer_report;
static uint8_t kb_status;

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) instance;
    (void) len;
    hid_report_id next_report = static_cast<hid_report_id>(report[0] + 1);
    switch (next_report) {
        case hid_report_id::KEYBOARD:
            tud_hid_report(static_cast<uint8_t>(hid_report_id::KEYBOARD), &kb_report, sizeof(kb_report));
            break;

        case hid_report_id::MOUSE:
            tud_hid_report(static_cast<uint8_t>(hid_report_id::MOUSE), &mouse_report, sizeof(mouse_report));
            break;

        case hid_report_id::CONSUMER:
            tud_hid_report(static_cast<uint8_t>(hid_report_id::CONSUMER), &consumer_report, sizeof(consumer_report));
            break;

        default:
            break;
    }
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == static_cast<uint8_t>(hid_report_id::KEYBOARD) && bufsize > 0) {
        kb_status = buffer[0];
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

/* USB HID abstractions */
USB::USB(void)
:
    hid_started(false)
{
    lock = spin_lock_init(spin_lock_claim_unused(true));
    tusb_init();
}

void USB::process(void) {
    if (!hid_started && connected() && tud_hid_ready()) {
        // send first message, rest handled by callback
        tud_hid_report(static_cast<uint8_t>(hid_report_id::KEYBOARD), &kb_report, sizeof(kb_report));
        tud_hid_report(static_cast<uint8_t>(hid_report_id::MOUSE), &mouse_report, sizeof(mouse_report));
        tud_hid_report(static_cast<uint8_t>(hid_report_id::CONSUMER), &consumer_report, sizeof(consumer_report));
        hid_started = true;
    } else {
        hid_started = false;
    }
    tud_task();
}

void USB::set_report(hid_keyboard_report_t& kb, hid_mouse_report_t& mouse, uint16_t& consumer) {
    uint32_t s = spin_lock_blocking(lock);
    kb_report = kb;
    mouse_report = mouse;
    consumer_report = consumer;
    spin_unlock(lock, s);
}

bool USB::connected(void) {
    return tud_mounted() && !tud_suspended();
}

uint8_t USB::led_status(void) {
    return kb_status;
}
