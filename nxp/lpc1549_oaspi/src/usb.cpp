#include <array>
#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "eth.h"
#include "oaspi.h"
#include "usr.h"
#include "usb.h"

/* private defines */
static constexpr uint8_t EP_NET_NOTIF = 0x81; // interrupt
static constexpr uint8_t EP_NET_OUT   = 0x02; // bulk
static constexpr uint8_t EP_NET_IN    = 0x82; // bulk
static constexpr uint8_t EP_HID_OUT   = 0x03; // interrupt
static constexpr uint8_t EP_HID_IN    = 0x83; // interrupt

// modified from usbd.h to set bmNetworkCapabilities=0x01 instead of 0x00
#undef  TUD_CDC_NCM_DESCRIPTOR
#define TUD_CDC_NCM_DESCRIPTOR(_itfnum, _desc_stridx, _mac_stridx, _ep_notif, _ep_notif_size, _epout, _epin, _epsize, _maxsegmentsize) \
    8, TUSB_DESC_INTERFACE_ASSOCIATION, _itfnum, 2, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_NETWORK_CONTROL_MODEL, 0, 0,\
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_NETWORK_CONTROL_MODEL, 0, _desc_stridx,\
    5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_HEADER, U16_TO_U8S_LE(0x0110),\
    5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_UNION, _itfnum, (uint8_t)((_itfnum) + 1),\
    13, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_ETHERNET_NETWORKING, _mac_stridx, 0, 0, 0, 0, U16_TO_U8S_LE(_maxsegmentsize), U16_TO_U8S_LE(0), 0, \
    6, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_NCM, U16_TO_U8S_LE(0x0100), 0x01, \
    7, TUSB_DESC_ENDPOINT, _ep_notif, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_ep_notif_size), 50,\
    9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum)+1), 0, 0, TUSB_CLASS_CDC_DATA, 0, NCM_DATA_PROTOCOL_NETWORK_TRANSFER_BLOCK, 0,\
    9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum)+1), 1, 2, TUSB_CLASS_CDC_DATA, 0, NCM_DATA_PROTOCOL_NETWORK_TRANSFER_BLOCK, 0,\
    7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0,\
    7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0

enum class string_id {
    LANGID,
    MANUFACTURER,
    PRODUCT,
    SERIAL_NUMBER,
    INTERFACE,
    MAC,
    COUNT
};

enum class iface_id {
    CDC,
    CDC_DATA,
    HID,
    COUNT
};

static const tusb_desc_device_t DEVICE_DESCRIPTOR = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0201,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xcafe,
    .idProduct          = 0x0069,
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(string_id::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(string_id::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(string_id::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

static const uint8_t HID_DESCRIPTOR[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE),
};

static const uint8_t CONFIG_DESCRIPTOR[] = {
    TUD_CONFIG_DESCRIPTOR(1, static_cast<uint8_t>(iface_id::COUNT), 0,
        TUD_CONFIG_DESC_LEN + TUD_CDC_NCM_DESC_LEN + TUD_HID_INOUT_DESC_LEN, 0, 500),
    TUD_CDC_NCM_DESCRIPTOR(static_cast<uint8_t>(iface_id::CDC), static_cast<uint8_t>(string_id::INTERFACE),
        static_cast<uint8_t>(string_id::MAC), EP_NET_NOTIF, 64, EP_NET_OUT, EP_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),
    TUD_HID_INOUT_DESCRIPTOR(static_cast<uint8_t>(iface_id::HID), 0, HID_ITF_PROTOCOL_NONE, sizeof(HID_DESCRIPTOR),
        EP_HID_OUT, EP_HID_IN, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static char const *STRING_DESCRIPTOR[] = { // keep <= 127 chars due to encoding
    [static_cast<size_t>(string_id::LANGID)]        = (const char[]) { 0x09, 0x04 }, // English (0x0409)
    [static_cast<size_t>(string_id::MANUFACTURER)]  = "miscboards",
    [static_cast<size_t>(string_id::PRODUCT)]       = "lpc1549_oaspi 10BASE-T1S/T1L",
    [static_cast<size_t>(string_id::SERIAL_NUMBER)] = "69420",
    [static_cast<size_t>(string_id::INTERFACE)]     = "lpc1549_oaspi 10BASE-T1S/T1L",
};

static constexpr uint8_t MS_OS_20_DESC_LEN = 0xB2;

static const uint8_t BOS_DESCRIPTOR[] = {
    TUD_BOS_DESCRIPTOR(TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN, 1),
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, 1),
};

static const uint8_t MS_OS_20_DESCRIPTOR[] = {
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A),
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), static_cast<uint8_t>(iface_id::CDC), 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08),
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'N', 'C', 'M', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
    U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A),
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
    'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    U16_TO_U8S_LE(0x0050),
    '{', 0x00, '1', 0x00, '2', 0x00, '3', 0x00, '4', 0x00, '5', 0x00, '6', 0x00, '7', 0x00, '8', 0x00, '-', 0x00,
    '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00,
    '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00,
    '8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00,
};
static_assert(sizeof(MS_OS_20_DESCRIPTOR) == MS_OS_20_DESC_LEN);

uint8_t tud_network_mac_address[6] = {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00};

/* private data */
static struct {
    USB *dev;
    std::array<eth::Packet*, eth::Eth::REQ_SIZE> reqs_buf;
    StaticQueue_t reqs_data;
    QueueHandle_t reqs;
    eth::Packet *pkt;
    StaticTask_t task_buffer;
    std::array<StackType_t, configMINIMAL_STACK_SIZE> task_stack;
} data;

/* private helpers */
static void usb_handler() {
    tud_int_handler(0);
}

static void usb_task(void*) {
    while (true) {
        tud_task();

        // send packet if available and able
        if (data.pkt == nullptr) {
            xQueueReceive(data.reqs, &data.pkt, 0);
        }
        if (data.pkt) {
            auto raw = data.pkt->raw();
            auto len = raw.size() - 4;
            if (tud_network_can_xmit(len)) {
                tud_network_xmit(raw.data(), len); // freed in tud_network_xmit_cb()
            }
        }
    }
    vTaskDelete(nullptr);
}

static void usb_eth_tx_renew(void*) {
    // driver not thread safe must run in tud_task()
    tud_network_recv_renew();
}

static void usb_eth_tx_cb() {
    // ready to try receiving from host again
    usbd_defer_func(usb_eth_tx_renew, nullptr, false);
}

static bool usb_eth_rx_cb(eth::Packet *pkt, void*) {
    if (xQueueSend(data.reqs, &pkt, 0) == pdTRUE) {
        // driver not thread safe must run in tud_task()
        usbd_defer_func(nullptr, nullptr, false);
        return true;
    } else {
        // OS doesn't query in time
        return false;
    }
}

/* public functions */
USB::USB(eth::OASPI &oaspi, eth::Eth &eth) : _oaspi(oaspi), _eth(eth) {
    configASSERT(data.dev == nullptr);
    data.dev  = this;
    data.reqs = xQueueCreateStatic(data.reqs_buf.size(), sizeof(eth::Packet*),
        reinterpret_cast<uint8_t*>(data.reqs_buf.data()), &data.reqs_data);
    data.pkt  = nullptr;
    configASSERT(data.reqs);
    tud_network_mac_address[5] = usr::id(); // randomize mac

    NVIC_SetVector(USB0_IRQn, reinterpret_cast<uint32_t>(usb_handler));
    NVIC_SetPriority(USB0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    Chip_USB_Init();

    tusb_init();
    configASSERT(xTaskCreateStatic(usb_task, "usb_task", data.task_stack.size(),
        nullptr, configMAX_PRIORITIES - 1, data.task_stack.data(), &data.task_buffer));
    eth.set_tx_cb(usb_eth_tx_cb);
    eth.set_rx_cb(usb_eth_rx_cb, nullptr);
}

USB::~USB() {
    configASSERT(false); // not supporting for now
}

uint8_t const *tud_descriptor_device_cb() {
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
    } else if (index == static_cast<uint8_t>(string_id::MAC)) {
        for (size_t i = 0; i < sizeof(tud_network_mac_address); i++) {
            str[1 + count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xF];
            str[1 + count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xF];
        }
    } else {
        const char *s = STRING_DESCRIPTOR[index];
        for (size_t i = 0; i < strlen(s); i++) {
            str[1 + i] = s[i]; // ASCII to UTF-16
        }
        count = strlen(s);
    }
    str[0] = (TUSB_DESC_STRING << 8) | (2 + 2 * count);
    return str;
}

uint8_t const *tud_descriptor_bos_cb() {
    return BOS_DESCRIPTOR;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
    if (stage != CONTROL_STAGE_SETUP) {
        return true;
    }
    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR && request->bRequest == 1) {
        if (request->wIndex == 7) {
            uint16_t total_len;
            std::memcpy(&total_len, MS_OS_20_DESCRIPTOR + 8, 2);
            return tud_control_xfer(rhport, request, const_cast<uint8_t*>(MS_OS_20_DESCRIPTOR), total_len);
        }
    }
    return false;
}

void tud_network_init_cb() {}

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
    if (size < eth::Packet::HDR_LEN || size > (eth::Packet::HDR_LEN + eth::Packet::MTU)) {
        // silently drop incorrectly sized packets
        return true;
    }
    // if can't send immediately, stall until next TX done which calls usb_eth_tx_cb()
    eth::Packet *pkt = data.dev->_eth.pkt_alloc(false);
    if (pkt) {
        std::memcpy(pkt->raw().data(), src, size);
        pkt->set_len(size - eth::Packet::HDR_LEN);
        return data.dev->_eth.send(pkt, false);
    }
    return false;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
    std::memcpy(dst, ref, arg);
    data.dev->_eth.pkt_free(data.pkt);
    data.pkt = nullptr;
    return arg;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void) instance;
    if (report_id != 0 || report_type != HID_REPORT_TYPE_OUTPUT || bufsize < 1) {
        return;
    }
    enum class op {
        REG_WRITE,
        REG_READ,
    };
    enum class ret {
        SUCCESS,
        ERROR_OP,
    };
    std::array<uint8_t, CFG_TUD_HID_EP_BUFSIZE> resp { static_cast<uint8_t>(ret::ERROR_OP) };
    switch (buffer[0]) {
        case static_cast<uint8_t>(op::REG_WRITE): {
            if (bufsize < 8) { return; }
            uint8_t  mms = buffer[1];
            uint16_t reg = (buffer[2] << 8) | buffer[3];
            uint32_t val = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
            while (!data.dev->_oaspi.reg_write(static_cast<eth::OASPI::MMS>(mms), reg, val)) {
                vTaskDelay(pdMS_TO_TICKS(5));
            }
            resp[0] = static_cast<uint8_t>(ret::SUCCESS);
            break;
        }

        case static_cast<uint8_t>(op::REG_READ): {
            if (bufsize < 4) { return; }
            uint8_t  mms = buffer[1];
            uint16_t reg = (buffer[2] << 8) | buffer[3];
            uint32_t val;
            while (true) {
                auto tmp = data.dev->_oaspi.reg_read(static_cast<eth::OASPI::MMS>(mms), reg);
                if (tmp.has_value()) {
                    val = tmp.value();
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(5));
            }
            resp[0] = static_cast<uint8_t>(ret::SUCCESS);
            resp[1] = (val >> 24) & 0xFF;
            resp[2] = (val >> 16) & 0xFF;
            resp[3] = (val >>  8) & 0xFF;
            resp[4] = (val >>  0) & 0xFF;
        }
    }
    tud_hid_report(0, resp.data(), resp.size());
}
