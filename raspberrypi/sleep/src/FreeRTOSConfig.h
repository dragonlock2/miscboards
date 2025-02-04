#pragma once

#include <FreeRTOSConfig_template.h>

// sleep library only handles core 0
#undef  configNUMBER_OF_CORES
#define configNUMBER_OF_CORES (1)
#undef  configUSE_CORE_AFFINITY

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (configMINIMAL_STACK_SIZE)
