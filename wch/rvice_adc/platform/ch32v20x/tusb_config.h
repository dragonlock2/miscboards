#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#define CFG_TUSB_MCU            (OPT_MCU_CH32V20X)
#define CFG_TUD_WCH_USBIP_USBFS (1)
#define CFG_TUSB_OS             (OPT_OS_FREERTOS)

#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENDPOINT0_SIZE  (64)

#define CFG_TUD_AUDIO           (1)
#define CFG_TUD_HID             (0)
#define CFG_TUD_CDC             (0)
#define CFG_TUD_MSC             (0)
#define CFG_TUD_MIDI            (0)
#define CFG_TUD_VENDOR          (0)

// macros for 8 channels
#define TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL_LEN (6 + 4 * (8 + 1))
#define TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL(_unitid, _srcid, _ch0master, _ch1, _ch2, _ch3, _ch4, _ch5, _ch6, _ch7, _ch8, _stridx) \
    TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL_LEN, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_FEATURE_UNIT, \
    _unitid, _srcid, U32_TO_U8S_LE(_ch0master), \
    U32_TO_U8S_LE(_ch1), U32_TO_U8S_LE(_ch2), U32_TO_U8S_LE(_ch3), U32_TO_U8S_LE(_ch4), \
    U32_TO_U8S_LE(_ch5), U32_TO_U8S_LE(_ch6), U32_TO_U8S_LE(_ch7), U32_TO_U8S_LE(_ch8), \
    _stridx

#define TUD_AUDIO_MIC_EIGHT_CH_DESC_LEN (TUD_AUDIO_DESC_IAD_LEN \
    + TUD_AUDIO_DESC_STD_AC_LEN \
    + TUD_AUDIO_DESC_CS_AC_LEN \
    + TUD_AUDIO_DESC_CLK_SRC_LEN \
    + TUD_AUDIO_DESC_INPUT_TERM_LEN \
    + TUD_AUDIO_DESC_OUTPUT_TERM_LEN \
    + TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL_LEN \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN \
    + TUD_AUDIO_DESC_STD_AS_INT_LEN \
    + TUD_AUDIO_DESC_CS_AS_INT_LEN \
    + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN \
    + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
    + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)

#define TUD_AUDIO_MIC_EIGHT_CH_DESCRIPTOR(_itfnum, _stridx, _nBytesPerSample, _nBitsUsedPerSample, _epin, _epsize) \
    TUD_AUDIO_DESC_IAD(_itfnum, 0x02, 0x00), \
    TUD_AUDIO_DESC_STD_AC(_itfnum, 0x00, _stridx), \
    TUD_AUDIO_DESC_CS_AC(0x0200, AUDIO_FUNC_MICROPHONE, \
        TUD_AUDIO_DESC_CLK_SRC_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN + TUD_AUDIO_DESC_OUTPUT_TERM_LEN + TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL_LEN, \
        AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS), \
    TUD_AUDIO_DESC_CLK_SRC(0x04, AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK, (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), 0x01, 0x00), \
    TUD_AUDIO_DESC_INPUT_TERM(0x01, AUDIO_TERM_TYPE_IN_GENERIC_MIC, 0x03, 0x04, 0x08, AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, 0x00, \
        AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, 0x00), \
    TUD_AUDIO_DESC_OUTPUT_TERM(0x03, AUDIO_TERM_TYPE_USB_STREAMING, 0x01, 0x02, 0x04, 0x0000, 0x00), \
    TUD_AUDIO_DESC_FEATURE_UNIT_EIGHT_CHANNEL(0x02, 0x01, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, \
        0x00), \
    TUD_AUDIO_DESC_STD_AS_INT(static_cast<uint8_t>((_itfnum) + 1), 0x00, 0x00, 0x00), \
    TUD_AUDIO_DESC_STD_AS_INT(static_cast<uint8_t>((_itfnum) + 1), 0x01, 0x01, 0x00), \
    TUD_AUDIO_DESC_CS_AS_INT(0x03, AUDIO_CTRL_NONE, AUDIO_FORMAT_TYPE_I, AUDIO_DATA_FORMAT_TYPE_I_PCM, 0x08, \
        AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, 0x00), \
    TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample), \
    TUD_AUDIO_DESC_STD_AS_ISO_EP(_epin, static_cast<uint8_t>(TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), \
        _epsize, 0x01), \
    TUD_AUDIO_DESC_CS_AS_ISO_EP(AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, AUDIO_CTRL_NONE, \
        AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, 0x0000)

// audio config
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE            (48000)
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX  (2)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX          (8)
#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN               (TUD_AUDIO_MIC_EIGHT_CH_DESC_LEN)

#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT               (1)
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ            (64)
#define CFG_TUD_AUDIO_ENABLE_EP_IN                  (1)
#define CFG_TUD_AUDIO_EP_IN_FLOW_CONTROL            (1)

#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX           (CFG_TUD_AUDIO_EP_SZ_IN)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ        (3 * CFG_TUD_AUDIO_EP_SZ_IN) // fifo size
#define CFG_TUD_AUDIO_EP_SZ_IN                      TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE, \
                                                        CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)

#endif // TUSB_CONFIG_H
