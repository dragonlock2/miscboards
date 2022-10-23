#define F_CPU 8000000L

#include <util/delay.h>
#include "mcc_generated_files/mcc.h"
#include "monitor.h"
#include "btns.h"
#include "gui.h"
#include "app.h"

void app_init() {
    monitor_init();
    btns_init();
    gui_init();
    printf("booted!\r\n");
}

void app_loop() {
    btns_loop();
    gui_loop();
}
