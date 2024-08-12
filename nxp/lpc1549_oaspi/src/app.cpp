#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include "usb.h"
#include "usr.h"

extern "C" void app_main(void*) {
    usb_init();
    printf("booted!\r\n");

    vTaskDelete(NULL);
}
