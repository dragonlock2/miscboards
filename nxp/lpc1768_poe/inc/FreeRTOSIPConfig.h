#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

#include <FreeRTOSIPConfig_template.h>

#undef  ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS (21) // fills up 32K

#undef  ipconfigSUPPORT_SELECT_FUNCTION
#define ipconfigSUPPORT_SELECT_FUNCTION  (1) // needed by mongoose

#undef  ipconfigIPv4_BACKWARD_COMPATIBLE
#define ipconfigIPv4_BACKWARD_COMPATIBLE (1) // needed by mongoose, disables IPv6

#endif /* FREERTOS_IP_CONFIG_H */