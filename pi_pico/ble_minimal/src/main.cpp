#include <tusb.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include "ble.h"

void thread0(void) {
    BLE ble;
    printf("thread0 booted!\r\n");

    while (true) {
        ble.process();
        sleep_us(1000); // appears to work well enough
    }
}

void thread1(void) {
    printf("thread1 booted!\r\n");

    int8_t i = 0;
    while (true) {
        printf("thread1 says hi %d\r\n", i++);
        sleep_ms(2500);
    }
}

int main(void) {
    stdio_init_all();
    while (!tud_cdc_connected());

    multicore_launch_core1(thread0); // cpu1
    thread1(); // cpu0
}
