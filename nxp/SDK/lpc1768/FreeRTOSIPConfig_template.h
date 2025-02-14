#pragma once

#include <stdio.h>

// options from https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html

#define ipconfigEVENT_QUEUE_LENGTH                  (ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5)
#define ipconfigIP_TASK_PRIORITY                    (configMAX_PRIORITIES - 2) // MAC task is highest
#define ipconfigIP_TASK_STACK_SIZE_WORDS            (configMINIMAL_STACK_SIZE * 4)
#define ipconfig_PROCESS_CUSTOM_ETHERNET_FRAMES     (0)
#define ipconfigUSE_NETWORK_EVENT_HOOK              (0)

#define ipconfigCHECK_IP_QUEUE_SPACE                (0)
#define ipconfigHAS_DEBUG_PRINTF                    (0)
#define ipconfigHAS_PRINTF                          (0)
#define ipconfigTCP_IP_SANITY                       (1)
#define ipconfigHAS_ROUTING_STATISTICS              (0)

#define ipconfigBUFFER_PADDING                      (0)
#define ipconfigPACKET_FILLER_SIZE                  (2)
#define ipconfigBYTE_ORDER                          (pdFREERTOS_LITTLE_ENDIAN)
#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM      (0)
#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM      (0)
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES (1) // driver allows unicast/multicast/broadcast
#define ipconfigETHERNET_DRIVER_FILTERS_PACKETS     (0)
#define ipconfigETHERNET_MINIMUM_PACKET_BYTES       (0) // driver pads to 60
#define ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES   (1)
#define ipconfigNETWORK_MTU                         (1500)
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS      (8)
#define ipconfigUSE_LINKED_RX_MESSAGES              (0)
#define ipconfigZERO_COPY_RX_DRIVER                 (1)
#define ipconfigZERO_COPY_TX_DRIVER                 (1)
#define ipconfigSUPPORT_NETWORK_DOWN_EVENT          (0)

#define ipconfigIGNORE_UNKNOWN_PACKETS              (0)
#define ipconfigTCP_HANG_PROTECTION                 (1)
#define ipconfigTCP_HANG_PROTECTION_TIME            (15) // s
#define ipconfigTCP_KEEP_ALIVE                      (1)
#define ipconfigTCP_KEEP_ALIVE_INTERVAL             (60) // s
#define ipconfigUSE_TCP                             (1)
#define ipconfigUSE_TCP_WIN                         (0)
#define ipconfigTCP_SRTT_MINIMUM_VALUE_MS           (200) // ms

#define ipconfigUDP_MAX_RX_PACKETS                  (4)
#define ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS       (pdMS_TO_TICKS(100))
#define ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS      (0)
#define ipconfigUDP_TIME_TO_LIVE                    (128)

#define ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND      (1)
#define ipconfigINCLUDE_FULL_INET_ADDR              (0)
#define ipconfigSELECT_USES_NOTIFY                  (0)
#define ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME     (portMAX_DELAY)
#define ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME        (portMAX_DELAY)
#define ipconfigSOCKET_HAS_USER_SEMAPHORE           (0)
#define ipconfigSUPPORT_SELECT_FUNCTION             (0)
#define ipconfigSUPPORT_SIGNALS                     (0)

#define ipconfigARP_CACHE_ENTRIES                   (16)
#define ipconfigARP_STORES_REMOTE_ADDRESSES         (0)
#define ipconfigARP_USE_CLASH_DETECTION             (1)
#define ipconfigMAX_ARP_AGE                         (150) // 1500s
#define ipconfigMAX_ARP_RETRANSMISSIONS             (5)
#define ipconfigUSE_ARP_REMOVE_ENTRY                (0)
#define ipconfigUSE_ARP_REVERSED_LOOKUP             (0)

#define ipconfigDHCP_FALL_BACK_AUTO_IP              (1)
#define ipconfigDHCP_REGISTER_HOSTNAME              (1)
#define ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY       (1)
#define ipconfigDNS_CACHE_ENTRIES                   (1)
#define ipconfigDNS_CACHE_NAME_LENGTH               (254)
#define ipconfigDNS_REQUEST_ATTEMPTS                (4)
#define ipconfigMAXIMUM_DISCOVER_TX_PERIOD          (pdMS_TO_TICKS(30000))
#define ipconfigUSE_DHCP                            (1)
#define ipconfigUSE_DHCPv6                          (0)
#define ipconfigUSE_DHCP_HOOK                       (0)
#define ipconfigUSE_DNS                             (1)
#define ipconfigUSE_DNS_CACHE                       (1)
#define ipconfigUSE_LLMNR                           (0)
#define ipconfigUSE_NBNS                            (0)
#define ipconfigUSE_MDNS                            (1)

#define ipconfigUSE_IPv4                            (1)
#define ipconfigUSE_IPv6                            (0)
#define ipconfigFORCE_IP_DONT_FRAGMENT              (0)
#define ipconfigICMP_TIME_TO_LIVE                   (64)
#define ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS     (1)
#define ipconfigREPLY_TO_INCOMING_PINGS             (1)
#define ipconfigSUPPORT_OUTGOING_PINGS              (0)

#define ipconfigND_CACHE_ENTRIES                    (16)

#define ipconfigUSE_RA                              (1)
#define ipconfigRA_SEARCH_COUNT                     (3)
#define ipconfigRA_IP_TEST_COUNT                    (3)

#define ipconfigIPv4_BACKWARD_COMPATIBLE            (0)

#if ipconfigHAS_DEBUG_PRINTF != 0
#define FreeRTOS_debug_printf(X) \
    do {                         \
        printf X ;               \
        printf("\r");            \
    } while (0)
#endif
