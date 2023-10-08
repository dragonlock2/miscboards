/**
 * C++ constructors/destructors
 * new? malloc
 * stl?
 * exceptions?
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ch32v00x.h>
#include "led.h"

static volatile bool flag = false;

__attribute__((interrupt))
static void ticker(void) {
    SysTick->SR = 0;
    flag = true;
}

int main(void) {
    SysTick_Start(SystemCoreClock / 2, ticker); // 0.5Hz
    __enable_irq();

    uint32_t id[3];
    UniqueID(id);
    printf("booted! unique id: 0x%08lx%08lx%08lx\r\n", id[0], id[1], id[2]);

    int8_t i = 69;
    while (true) {
        led_toggle();
        printf("tick! %d\r\n", i++);

        flag = false;
        while (!flag);
    }
}
