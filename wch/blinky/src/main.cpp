#include <stdbool.h>
#include <stdio.h>
#include "led.h"
#include "ticker.h"

static volatile bool flag = false;
static void ticker(void) {
    flag = true;
}

extern "C" int main(void) {
    ticker_start(SystemCoreClock / 2, ticker); // 0.5Hz
    __enable_irq();

    uint32_t id[3]; UniqueID(id);
    printf("booted! flash size: %dKiB, unique id: 0x%08lx%08lx%08lx\r\n", FlashSize(), id[0], id[1], id[2]);

    int8_t i = 69;
    while (true) {
        led_toggle();
        printf("tick! %d\r\n", i++);

        flag = false;
        while (!flag);
    }
}
