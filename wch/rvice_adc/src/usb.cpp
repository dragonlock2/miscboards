#include <array>
#include <cstring>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "fpga.h"
#include "usb.h"

/* private constants */
enum class usb_string_desc {
    LANGID,
    MANUFACTURER,
    PRODUCT,
    SERIAL_NUMBER,
    COUNT
};

enum class usb_itf_num {
    VENDOR,
    AUDIO_CONTROL,
    AUDIO_STREAMING,
    COUNT
};

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110, // USB 1.1 (not high speed)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x0069,
    .idProduct          = 0x0421, // 0x0420 used by JABI
    .bcdDevice          = 0x0000,
    .iManufacturer      = static_cast<uint8_t>(usb_string_desc::MANUFACTURER),
    .iProduct           = static_cast<uint8_t>(usb_string_desc::PRODUCT),
    .iSerialNumber      = static_cast<uint8_t>(usb_string_desc::SERIAL_NUMBER),
    .bNumConfigurations = 0x01,
};

#define TUD_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO_FUNC_1_DESC_LEN + TUD_VENDOR_DESC_LEN)
#define TUD_VENDOR_EPNUM (1)
#define TUD_AUDIO_EPNUM  (3) // only ep3 supports 1023 byte packets

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, static_cast<uint8_t>(usb_itf_num::COUNT), 0, TUD_CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100), // mA
    TUD_VENDOR_DESCRIPTOR(static_cast<uint8_t>(usb_itf_num::VENDOR), 0, TUD_VENDOR_EPNUM, TUSB_DIR_IN_MASK | TUD_VENDOR_EPNUM, 64),
    TUD_AUDIO_MIC_EIGHT_CH_DESCRIPTOR(static_cast<uint8_t>(usb_itf_num::AUDIO_CONTROL), 0,
        CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * 8,
        TUSB_DIR_IN_MASK | TUD_AUDIO_EPNUM, CFG_TUD_AUDIO_EP_SZ_IN),
};

static char const* desc_strings[] = { // keep <= 127 chars due to encoding
    [static_cast<size_t>(usb_string_desc::LANGID)]        = (const char[]) { 0x09, 0x04 }, // supported language is English (0x0409)
    [static_cast<size_t>(usb_string_desc::MANUFACTURER)]  = "dragonlock2",
    [static_cast<size_t>(usb_string_desc::PRODUCT)]       = "rvice_adc",
    [static_cast<size_t>(usb_string_desc::SERIAL_NUMBER)] = "not null",
};

namespace rpc {

/* private constants */
enum class rpc_id {
    FPGA_OFF,
    FPGA_ON,
    FLASH_ERASE,
    FLASH_WRITE,
    FLASH_READ,
};

enum class rpc_ret {
    SUCCESS,
    FAIL,
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
static void start_receive(uint8_t rhport) {
    usbd_edpt_xfer(rhport, data.ep_out, reinterpret_cast<uint8_t*>(&data.pkt), sizeof(data.pkt));
}

static void start_send(uint8_t rhport) {
    usbd_edpt_xfer(rhport, data.ep_in, reinterpret_cast<uint8_t*>(&data.pkt), sizeof(data.pkt));
}

static void init(void) {
    memset(&data.pkt, 0, sizeof(data.pkt));
}

static void reset(uint8_t rhport) {
    (void) rhport;
    init();
}

static uint16_t open(uint8_t rhport, tusb_desc_interface_t const* desc_intf, uint16_t max_len) {
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

        start_receive(rhport);
    }
    return (uintptr_t) p_desc - (uintptr_t) desc_intf;
}

static bool xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
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
            start_send(rhport);
        } else {
            start_receive(rhport);
        }
    } else if (ep_addr == data.ep_in) {
        start_receive(rhport);
    }
    return true;
}

};

namespace audio {

/* private data */
static struct {
    uint32_t sample_freq;
    uint8_t  clk_valid;
    bool     mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];
    uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];
    audio_control_range_4_n_t(1) sample_freq_range;

    // TODO update
    uint16_t test_buffer_audio[CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];
    uint16_t startVal;
} data;

/* public functions */
extern "C" bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request, uint8_t* pBuff) {
    (void) rhport;
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);
    
    const uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    const uint8_t ctrlSel    = TU_U16_HIGH(p_request->wValue);
    const uint8_t entityID   = TU_U16_HIGH(p_request->wIndex);

    if (entityID == 2) {
        switch (ctrlSel) {
            case AUDIO_FU_CTRL_MUTE:
                TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));
                data.mute[channelNum] = reinterpret_cast<audio_control_cur_1_t*>(pBuff)->bCur;
                return true;
                break;

            case AUDIO_FU_CTRL_VOLUME:
                TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));
                data.volume[channelNum] = (uint16_t) ((audio_control_cur_2_t*) pBuff)->bCur;
                return true;
                break;
        }
    }
    return false;
}

extern "C" bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request) {
    const uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    const uint8_t ctrlSel    = TU_U16_HIGH(p_request->wValue);
    const uint8_t entityID   = TU_U16_HIGH(p_request->wIndex);

    if (entityID == 1) {
        switch (ctrlSel) {
            case AUDIO_TE_CTRL_CONNECTOR: {
                audio_desc_channel_cluster_t ret = { // dummy values
                    .bNrChannels     = 1,
                    .bmChannelConfig = AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
                    .iChannelNames   = 0,
                };
                return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request,
                    static_cast<void*>(&ret), sizeof(ret));
                break;
            }
        }
    } else if (entityID == 2) { // mute/volume status
        switch (ctrlSel) {
            case AUDIO_FU_CTRL_MUTE:
                return tud_control_xfer(rhport, p_request, &data.mute[channelNum], 1);
                break;

            case AUDIO_FU_CTRL_VOLUME:
                switch (p_request->bRequest) {
                    case AUDIO_CS_REQ_CUR:
                        return tud_control_xfer(rhport, p_request,
                            &data.volume[channelNum], sizeof(data.volume[channelNum]));
                        break;

                    case AUDIO_CS_REQ_RANGE: {
                        audio_control_range_2_n_t(1) ret = {
                            .wNumSubRanges = 1,
                            .subrange = {{
                                .bMin = -90, // dB
                                .bMax = 90,  // dB
                                .bRes = 1,   // dB
                            }},
                        };
                        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request,
                            reinterpret_cast<void*>(&ret), sizeof(ret));
                        break;
                    }
                }
                break;
        }
    } else if (entityID == 4) { // sample frequency status
        switch (ctrlSel) {
            case AUDIO_CS_CTRL_SAM_FREQ:
                switch (p_request->bRequest) {
                    case AUDIO_CS_REQ_CUR:
                        return tud_control_xfer(rhport, p_request,
                            &data.sample_freq, sizeof(data.sample_freq));
                        break;

                    case AUDIO_CS_REQ_RANGE:
                        return tud_control_xfer(rhport, p_request,
                            &data.sample_freq_range, sizeof(data.sample_freq_range));
                        break;
                }
                break;

            case AUDIO_CS_CTRL_CLK_VALID:
                return tud_control_xfer(rhport, p_request, &data.clk_valid, sizeof(data.clk_valid));
                break;
        }
    }
    return false;
}

extern "C" bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting) {
    (void) rhport;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    // TODO update
    tud_audio_write((uint8_t*) data.test_buffer_audio, sizeof(data.test_buffer_audio));

    return true;
}

extern "C" bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting) {
    (void) rhport;
    (void) n_bytes_copied;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    // TODO update
    for (size_t cnt = 0; cnt < (CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX); cnt++) {
        data.test_buffer_audio[cnt] = data.startVal++;
    }

    return true;
}

};

static void usb_handler(void) {
    tud_int_handler(0);
}

/* public functions */
void usb_task(void* args) {
    (void) args;
    NVIC_SetVector(USBHD_IRQn, usb_handler);
    tusb_init();

    // audio defaults
    audio::data.sample_freq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
    audio::data.clk_valid   = 1;
    audio::data.sample_freq_range.wNumSubRanges    = 1;
    audio::data.sample_freq_range.subrange[0].bMin = audio::data.sample_freq;
    audio::data.sample_freq_range.subrange[0].bMax = audio::data.sample_freq;
    audio::data.sample_freq_range.subrange[0].bRes = 0;

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
            .init             = rpc::init,
            .reset            = rpc::reset,
            .open             = rpc::open,
            .control_xfer_cb  = NULL,
            .xfer_cb          = rpc::xfer_cb,
            .sof              = NULL,
        },
    };
    *driver_count = TU_ARRAY_SIZE(app_drivers);
    return app_drivers;
}
