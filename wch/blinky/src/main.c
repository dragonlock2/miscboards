/**
 * fast interrupts mret, hpe
 * C++ support
 * constructors!
 * destructors!
 * new?
 * stl?
 * exceptions?
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "ch32v00x.h"
#include "core_riscv.h"
#include "system_ch32v00x.h"
#include "ch32v00x_gpio.h"
#include "ch32v00x_rcc.h"

static volatile bool flag = false;

__attribute__((interrupt))
static void ticker(void) {
    SysTick->SR = 0;
    flag = true;
}

int main(void) {
    GPIO_InitTypeDef led = {
        .GPIO_Pin   = GPIO_Pin_0,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOD, &led);
    SysTick_Start(SystemCoreClock / 2, ticker); // 0.5Hz
    __enable_irq();

    printf("booted! unique id: 0x%08lx%08lx%08lx\r\n",
        *(uint32_t*) 0x1FFFF7F0,
        *(uint32_t*) 0x1FFFF7EC,
        *(uint32_t*) 0x1FFFF7E8
    );

    int8_t i = 69;
    while (true) {
        GPIO_WriteBit(GPIOD, GPIO_Pin_0, i % 2 ? Bit_RESET : Bit_SET);
        printf("tick! %d\r\n", i++);

        flag = false;
        while (!flag);
    }
}
