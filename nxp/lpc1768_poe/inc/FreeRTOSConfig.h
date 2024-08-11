#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <FreeRTOSConfig_template.h>

#undef  configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE (20 * 1024)

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (configMINIMAL_STACK_SIZE * 4) // needed by mongoose

#endif /* FREERTOS_CONFIG_H */
