#define F_CPU 16000000L

#include <util/delay.h>
#include "mcc_generated_files/mcc.h"
#include "ssd1306.h"
#include "app.h"

void app_init() {
    ssd1306_init();
    printf("booted!\r\n");
}

void app_loop() {
    while (true) {
        static int8_t c = 0;
        ssd1306_clear_display();
        ssd1306_set_cursor(abs(c++), 0);
        ssd1306_draw_string("hello world!");
        ssd1306_display();
    }
}
