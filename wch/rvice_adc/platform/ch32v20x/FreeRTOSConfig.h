#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <FreeRTOSConfig_template.h>

#undef  configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE (5 * 1024)

#define configAPP_MAIN_PRIORITY (1) // low-priority

#endif // FREERTOS_CONFIG_H
