#pragma once

#include <FreeRTOSConfig_template.h>

#define configAPP_MAIN_PRIORITY   (tskIDLE_PRIORITY + 1)
#define configAPP_MAIN_STACK_SIZE (4096) // need more for BTstack

#define configNOTIF_BLE (0)
