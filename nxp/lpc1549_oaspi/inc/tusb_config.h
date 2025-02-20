#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#define CFG_TUSB_OS            (OPT_OS_FREERTOS)
#define CFG_TUSB_MCU           (OPT_MCU_LPC15XX)
#define CFG_TUSB_MEM_ALIGN     TU_ATTR_ALIGNED(64) // required by driver

#define CFG_TUSB_RHPORT0_MODE  (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENDPOINT0_SIZE (64)

#define CFG_TUD_NCM            (1)
#define CFG_TUD_HID            (1)

#define CFG_TUD_NCM_IN_NTB_MAX_SIZE  (2048)
#define CFG_TUD_NCM_OUT_NTB_MAX_SIZE (2048)
#define CFG_TUD_NCM_IN_NTB_N         (3)
#define CFG_TUD_NCM_OUT_NTB_N        (2)

#endif // TUSB_CONFIG_H
