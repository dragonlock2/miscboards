#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <FreeRTOSConfig_template.h>

#undef  configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE (5 * 1024)

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (256)

// tasks can't block on bits, must use entire index
#define configNOTIF_USB_ETH (0) // task never uses SPI
#define configNOTIF_SPI     (0)
#define configNOTIF_ETH     (1)

#endif /* FREERTOS_CONFIG_H */
