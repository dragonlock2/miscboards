#include <ch32v20x.h>
#include "tick.h"

/* private data */
static void (*tick_cb)(void) = nullptr;

/* private helpers */
__attribute__((interrupt))
static void tick_isr(void) {
    SysTick->SR = 0;
    if (tick_cb) {
        tick_cb();
    }
}

/* public functions */
void tick_init(uint64_t freq, void (*cb)(void)) {
    NVIC_SetVector(SysTicK_IRQn, tick_isr);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = SystemCoreClock / freq - 1;
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
    tick_cb = cb;
    __enable_irq();
}
