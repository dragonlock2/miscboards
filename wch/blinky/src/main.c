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

volatile bool flag = false;

__attribute__((interrupt))
void ticker(void) {
    SysTick->SR = 0;
    flag = true;
}

int main(void) {
    NVIC_SetVector(SysTicK_IRQn, ticker);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = SystemCoreClock / 2 - 1; // 0.5Hz
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
    __enable_irq();

    printf("booted! unique id: 0x%08lx%08lx%08lx\r\n",
        *(uint32_t*) 0x1FFFF7F0,
        *(uint32_t*) 0x1FFFF7EC,
        *(uint32_t*) 0x1FFFF7E8
    );

    int8_t i = 69;
    while (true) {
        printf("tick! %d\r\n", i++);

        flag = false;
        while (!flag);
    }
    return 0;
}
