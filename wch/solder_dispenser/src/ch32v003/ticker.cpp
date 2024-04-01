#include <ch32v00x.h>
#include "ticker.h"

static volatile bool* signal = nullptr;

__attribute__((interrupt))
static void ticker_isr(void) {
    SysTick->SR = 0;
    if (signal) {
        *signal = true;
    }
}

void ticker_start(uint32_t freq, volatile bool* sig) {
    NVIC_SetVector(SysTicK_IRQn, ticker_isr);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = (SystemCoreClock / freq) - 1;
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
    signal = sig;
    __enable_irq();
}
