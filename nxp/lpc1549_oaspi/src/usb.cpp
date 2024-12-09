#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>
#include "eth.h"
#include "oaspi.h"
#include "usr.h"
#include "usb.h"

/* private defines */
#define EP_NET_NOTIF (0x81) // interrupt
#define EP_NET_OUT   (0x02) // bulk
#define EP_NET_IN    (0x82) // bulk
#define EP_HID       (0x04) // interrupt

enum class usb_string {
    LANGID,
    MANUFACTURER,
    PRODUCT,
    SERIAL_NUMBER,
    INTERFACE,
    MAC,
    COUNT
};

enum class usb_itf {
    CDC,
    CDC_DATA,
    HID,
    COUNT
};

enum class usb_config {
    RNDIS,
    ECM,
    COUNT
};

static const tusb_desc_device_t device_descriptor = {
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
    .iManufacturer      = static_cast<uint8_t>(usb_string::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(usb_string::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(usb_string::SERIAL_NUMBER),
    .bNumConfigurations = static_cast<uint8_t>(usb_config::COUNT),
};

static const uint8_t hid_descriptor[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE),
};

static const uint8_t rndis_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(static_cast<uint8_t>(usb_config::RNDIS) + 1, static_cast<uint8_t>(usb_itf::COUNT), 0,
        TUD_CONFIG_DESC_LEN + TUD_RNDIS_DESC_LEN + TUD_HID_INOUT_DESC_LEN, 0, 500),
    TUD_RNDIS_DESCRIPTOR(static_cast<uint8_t>(usb_itf::CDC), static_cast<uint8_t>(usb_string::INTERFACE),
        EP_NET_NOTIF, 8, EP_NET_OUT, EP_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE),
    TUD_HID_INOUT_DESCRIPTOR(static_cast<uint8_t>(usb_itf::HID), 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_descriptor),
        EP_HID, 0x80 | EP_HID, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t ecm_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(static_cast<uint8_t>(usb_config::ECM) + 1, static_cast<uint8_t>(usb_itf::COUNT), 0,
        TUD_CONFIG_DESC_LEN + TUD_CDC_ECM_DESC_LEN + TUD_HID_INOUT_DESC_LEN, 0, 500),
    TUD_CDC_ECM_DESCRIPTOR(static_cast<uint8_t>(usb_itf::CDC), static_cast<uint8_t>(usb_string::INTERFACE),
        static_cast<uint8_t>(usb_string::MAC), EP_NET_NOTIF, 64, EP_NET_OUT, EP_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),
    TUD_HID_INOUT_DESCRIPTOR(static_cast<uint8_t>(usb_itf::HID), 0, HID_ITF_PROTOCOL_NONE, sizeof(hid_descriptor),
        EP_HID, 0x80 | EP_HID, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static char const *desc_strings[] = { // keep <= 127 chars due to encoding
    [static_cast<size_t>(usb_string::LANGID)]        = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
    [static_cast<size_t>(usb_string::MANUFACTURER)]  = "miscboards",
    [static_cast<size_t>(usb_string::PRODUCT)]       = "lpc1549_oaspi",
    [static_cast<size_t>(usb_string::SERIAL_NUMBER)] = "69420",
    [static_cast<size_t>(usb_string::INTERFACE)]     = "lpc1649_oaspi interface",
};

// intentional non-static, non-const
uint8_t tud_network_mac_address[6] = {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00};

/* private data */
static struct {
    USB *dev;
    TaskHandle_t usb_eth;
    QueueHandle_t reqs;
    eth::Packet *pkt;
    uint32_t tx_drop, rx_drop;
} data;

/* private helpers */
static void usb_handler(void) {
    tud_int_handler(0);
}

static void usb_task(void*) {
    while (true) {
        tud_task();
        xTaskNotifyGiveIndexed(data.usb_eth, configNOTIF_USB_ETH);
    }
    vTaskDelete(nullptr);
}

static void usb_eth_task(void*) {
    while (true) {
        xQueueReceive(data.reqs, &data.pkt, portMAX_DELAY);
        while (!tud_network_can_xmit(0)) {
            // driver doesn't have callback when can_xmit=true, so must try on every event
            ulTaskNotifyTakeIndexed(configNOTIF_USB_ETH, true, pdMS_TO_TICKS(10));
        }
        auto raw = data.pkt->raw();
        tud_network_xmit(raw.data(), raw.size() - 4); // freed in callback below
    }
    vTaskDelete(nullptr);
}

static bool usb_eth_cb(eth::Packet *pkt, void *arg) {
    (void) arg;
    if (xQueueSend(data.reqs, &pkt, 0) == pdTRUE) {
        return true;
    } else {
        data.rx_drop++;
        return false;
    }
}

/* public functions */
USB::USB(eth::OASPI &oaspi, eth::Eth &dev) : oaspi(oaspi), dev(dev) {
    configASSERT(data.dev == nullptr);
    data.dev  = this;
    data.reqs = xQueueCreate(eth::POOL_SIZE / 2, sizeof(eth::Packet*));
    configASSERT(data.reqs);
    tud_network_mac_address[5] = usr::id(); // randomize mac

    NVIC_SetVector(USB0_IRQn, reinterpret_cast<uint32_t>(usb_handler));
    NVIC_SetPriority(USB0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    Chip_USB_Init();

    tusb_init();
    configASSERT(xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, nullptr, configMAX_PRIORITIES - 1, nullptr) == pdPASS);
    configASSERT(xTaskCreate(usb_eth_task, "usb_eth_task", configMINIMAL_STACK_SIZE, nullptr, configMAX_PRIORITIES - 2, &data.usb_eth) == pdPASS);
    dev.set_cb(0, usb_eth_cb, nullptr);
}

USB::~USB() {
    configASSERT(false); // not supporting for now
}

std::tuple<uint32_t, uint32_t> USB::get_error(void) {
    return {data.tx_drop, data.rx_drop};
}

uint8_t const *tud_descriptor_device_cb(void) {
    return reinterpret_cast<uint8_t const*>(&device_descriptor);
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return hid_descriptor;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    switch (static_cast<usb_config>(index)) {
        case usb_config::RNDIS: return rndis_configuration;
        case usb_config::ECM:   return ecm_configuration;
        default: return nullptr;
    }
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t str[127]; // UTF-16
    (void) langid;

    // assumes little-endian!
    uint8_t count = 0;
    if (index >= static_cast<uint8_t>(usb_string::COUNT)) {
        return nullptr;
    } else if (index == static_cast<uint8_t>(usb_string::LANGID)) {
        std::memcpy(&str[1], desc_strings[index], strlen(desc_strings[index]));
        count = 1;
    } else if (index == static_cast<uint8_t>(usb_string::MAC)) {
        for (size_t i = 0; i < sizeof(tud_network_mac_address); i++) {
            str[1 + count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xF];
            str[1 + count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xF];
        }
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

void tud_network_init_cb(void) {}

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
    if (size < eth::Packet::HDR_LEN || size > (eth::Packet::HDR_LEN + eth::Packet::MTU)) {
        return false;
    }
    eth::Packet *pkt = data.dev->dev.pkt_alloc(false);
    if (pkt) {
        std::memcpy(pkt->raw().data(), src, size);
        pkt->set_len(size - eth::Packet::HDR_LEN);
        data.dev->dev.send(pkt);
        tud_network_recv_renew();
        return true;
    } else {
        data.tx_drop++;
        return false;
    }
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
    std::memcpy(dst, ref, arg);
    data.dev->dev.pkt_free(data.pkt);
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
    (void) report_id;
    (void) report_type;
#ifdef CONFIG_NCN26010
    if (bufsize == 3) {
        bool enable = buffer[0] & 0x01;
        bool leader = buffer[0] & 0x02;
        uint8_t id  = buffer[1];
        uint8_t cnt = buffer[2];
        if (enable) {
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0x8002, leader ? 0x0003 : 0x0000);
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0xCA01, 0x8000);
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0xCA02, (cnt << 8) | id);
        } else {
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0x8002, 0x0000);
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0xCA01, 0x0000);
            data.dev->oaspi.reg_write(eth::oaspi_mms::PHY_PLCA, 0xCA02, 0x08FF);
        }
    }
#else
    (void) buffer;
    (void) bufsize;
#endif
}
