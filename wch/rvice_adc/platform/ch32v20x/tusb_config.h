#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#define CFG_TUSB_MCU (OPT_MCU_CH32V20X)
#define CFG_TUSB_OS  (OPT_OS_FREERTOS)

#define CFG_TUSB_RHPORT0_MODE  (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENDPOINT0_SIZE (64)

#define CFG_TUD_HID            (1)
#define CFG_TUD_CDC            (0)
#define CFG_TUD_MSC            (0)
#define CFG_TUD_MIDI           (0)
#define CFG_TUD_VENDOR         (0)

#define CFG_TUD_HID_EP_BUFSIZE (16)

#endif // TUSB_CONFIG_H
