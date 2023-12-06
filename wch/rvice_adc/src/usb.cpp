#include <array>
#include <cstring>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "fpga.h"
#include "usb.h"

/* private constants */
enum class usb_string_desc {
    LANGID        = 0x00,
    MANUFACTURER  = 0x01,
    PRODUCT       = 0x02,
    SERIAL_NUMBER = 0x03,
    COUNT
};

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110, // USB 1.1 (not high speed)
    .bDeviceClass       = TUSB_CLASS_VENDOR_SPECIFIC,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x0069,
    .idProduct          = 0x0421, // 0x0420 used by JABI
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(usb_string_desc::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(usb_string_desc::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(usb_string_desc::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN), TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500), // 500mA
    TUD_VENDOR_DESCRIPTOR(0, 0, 0x01, 0x81, CFG_TUD_ENDPOINT0_SIZE), // ep1
};

static char const* desc_strings[] = { // keep <= 127 chars due to encoding
    [static_cast<size_t>(usb_string_desc::LANGID)]        = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
    [static_cast<size_t>(usb_string_desc::MANUFACTURER)]  = "dragonlock2",
    [static_cast<size_t>(usb_string_desc::PRODUCT)]       = "rvice_adc",
    [static_cast<size_t>(usb_string_desc::SERIAL_NUMBER)] = "null",
};

enum class rpc_id {
    FPGA_OFF = 0,
    FPGA_ON = 1,
    FLASH_ERASE = 2,
    FLASH_WRITE = 3,
    FLASH_READ = 4,
};

enum class rpc_ret {
    SUCCESS = 0,
    FAIL = 1,
};

/* private data */
static struct {
    uint8_t ep_out, ep_in;
    CFG_TUSB_MEM_ALIGN struct {
        union {
            rpc_id id;
            rpc_ret ret;
        };
        uint32_t addr;
        uint32_t len;
        std::array<uint8_t, PAGE_SIZE> data;
    } pkt;
} data;

static_assert(sizeof(data.pkt) == (12 + PAGE_SIZE)); // extremely lazy rpc :P

/* private helpers */
static void rpc_start_receive(uint8_t rhport) {
    usbd_edpt_xfer(rhport, data.ep_out, reinterpret_cast<uint8_t*>(&data.pkt), sizeof(data.pkt));
}

static void rpc_start_send(uint8_t rhport) {
    usbd_edpt_xfer(rhport, data.ep_in, reinterpret_cast<uint8_t*>(&data.pkt), sizeof(data.pkt));
}

static void rpc_init(void) {
    memset(&data.pkt, 0, sizeof(data.pkt));
}

static void rpc_reset(uint8_t rhport) {
    (void) rhport;
    rpc_init();
}

static uint16_t rpc_open(uint8_t rhport, tusb_desc_interface_t const* desc_intf, uint16_t max_len) {
    TU_VERIFY(desc_intf->bInterfaceClass == TUSB_CLASS_VENDOR_SPECIFIC, 0);
    const uint8_t* p_desc = tu_desc_next(desc_intf);
    const uint8_t* desc_end = p_desc + max_len;
    if (desc_intf->bNumEndpoints) {
        while (tu_desc_type(p_desc) != TUSB_DESC_ENDPOINT && p_desc < desc_end) {
            p_desc = tu_desc_next(p_desc);
        }
        TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_intf->bNumEndpoints,
            TUSB_XFER_BULK, &data.ep_out, &data.ep_in), 0);
        p_desc += desc_intf->bNumEndpoints * sizeof(tusb_desc_endpoint_t);

        usbd_edpt_claim(rhport, data.ep_out);
        usbd_edpt_claim(rhport, data.ep_in);

        rpc_start_receive(rhport);
    }
    return (uintptr_t) p_desc - (uintptr_t) desc_intf;
}

static bool rpc_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    if (ep_addr == data.ep_out) {
        if (result == XFER_RESULT_SUCCESS && xferred_bytes == sizeof(data.pkt)) {
            rpc_ret ret = rpc_ret::FAIL; // careful it's unioned
            switch (data.pkt.id) {
                case rpc_id::FPGA_OFF:
                    fpga_off();
                    ret = rpc_ret::SUCCESS;
                    break;

                case rpc_id::FPGA_ON:
                    fpga_on();
                    ret = rpc_ret::SUCCESS;
                    break;

                case rpc_id::FLASH_ERASE:
                    if (fpga_erase(data.pkt.addr)) {
                        ret = rpc_ret::SUCCESS;
                    }
                    break;

                case rpc_id::FLASH_WRITE:
                    if (fpga_write(data.pkt.addr, data.pkt.data)) {
                        ret = rpc_ret::SUCCESS;
                    }
                    break;

                case rpc_id::FLASH_READ:
                    if (fpga_read(data.pkt.addr, data.pkt.data)) {
                        ret = rpc_ret::SUCCESS;
                    }
                    break;
            }
            data.pkt.ret = ret;
            rpc_start_send(rhport);
        } else {
            rpc_start_receive(rhport);
        }
    } else if (ep_addr == data.ep_in) {
        rpc_start_receive(rhport);
    }
    return true;
}

static void usb_handler(void) {
    tud_int_handler(0);
}

/* public functions */
void usb_task(void* args) {
    (void) args;
    NVIC_SetVector(USBHD_IRQn, usb_handler);
    tusb_init();
    while (1) {
        tud_task();
    }
    vTaskDelete(NULL);
}

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &desc_device;
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

usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t* driver_count) {
    static const usbd_class_driver_t app_drivers[] = {
        {
            .init             = rpc_init,
            .reset            = rpc_reset,
            .open             = rpc_open,
            .control_xfer_cb  = NULL,
            .xfer_cb          = rpc_xfer_cb,
            .sof              = NULL,
        },
    };
    *driver_count = TU_ARRAY_SIZE(app_drivers);
    return app_drivers;
}
