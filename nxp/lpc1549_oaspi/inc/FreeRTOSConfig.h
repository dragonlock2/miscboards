#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <FreeRTOSConfig_template.h>

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (configMINIMAL_STACK_SIZE)

#endif /* FREERTOS_CONFIG_H */
