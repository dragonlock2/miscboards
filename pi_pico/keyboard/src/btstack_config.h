#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// mostly taken from https://github.com/raspberrypi/pico-examples/blob/master/pico_w/bt/config/btstack_config.h

#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_CENTRAL
#define ENABLE_L2CAP_LE_CREDIT_BASED_FLOW_CONTROL_MODE
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_PRINTF_HEXDUMP
#define ENABLE_SCO_OVER_HCI

#define HCI_OUTGOING_PRE_BUFFER_SIZE              (4)
#define HCI_ACL_PAYLOAD_SIZE                      (1691 + 4)
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT              (4)
#define MAX_NR_AVDTP_CONNECTIONS                  (1)
#define MAX_NR_AVDTP_STREAM_ENDPOINTS             (1)
#define MAX_NR_AVRCP_CONNECTIONS                  (2)
#define MAX_NR_BNEP_CHANNELS                      (1)
#define MAX_NR_BNEP_SERVICES                      (1)
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES (2)
#define MAX_NR_GATT_CLIENTS                       (1)
#define MAX_NR_HCI_CONNECTIONS                    (2)
#define MAX_NR_HID_HOST_CONNECTIONS               (1)
#define MAX_NR_HIDS_CLIENTS                       (1)
#define MAX_NR_HFP_CONNECTIONS                    (1)
#define MAX_NR_L2CAP_CHANNELS                     (4)
#define MAX_NR_L2CAP_SERVICES                     (3)
#define MAX_NR_RFCOMM_CHANNELS                    (1)
#define MAX_NR_RFCOMM_MULTIPLEXERS                (1)
#define MAX_NR_RFCOMM_SERVICES                    (1)
#define MAX_NR_SERVICE_RECORD_ITEMS               (4)
#define MAX_NR_SM_LOOKUP_ENTRIES                  (3)
#define MAX_NR_WHITELIST_ENTRIES                  (16)
#define MAX_NR_LE_DEVICE_DB_ENTRIES               (16)

#define MAX_NR_CONTROLLER_ACL_BUFFERS (3)
#define MAX_NR_CONTROLLER_SCO_PACKETS (3)

#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL
#define HCI_HOST_ACL_PACKET_LEN (1024)
#define HCI_HOST_ACL_PACKET_NUM (3)
#define HCI_HOST_SCO_PACKET_LEN (120)
#define HCI_HOST_SCO_PACKET_NUM (3)

#define NVM_NUM_DEVICE_DB_ENTRIES (16)
#define NVM_NUM_LINK_KEYS         (16)

#define MAX_ATT_DB_SIZE (512)

#define HAVE_EMBEDDED_TIME_MS
#define HAVE_ASSERT

#define HCI_RESET_RESEND_TIMEOUT_MS (1000)

#define ENABLE_SOFTWARE_AES128
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS

#endif // BTSTACK_CONFIG_H
