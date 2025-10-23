#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <FreeRTOSConfig_template.h>

#undef  configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE (128) // enough for printf

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (configMINIMAL_STACK_SIZE)

#define configETH_PRIORITY   (configMAX_PRIORITIES - 2)
#define configETH_STACK_SIZE (configMINIMAL_STACK_SIZE * 2)

// tasks can't block on bits, must use entire index
#define configNOTIF_SPI (0)
#define configNOTIF_ETH (1)

#endif /* FREERTOS_CONFIG_H */
