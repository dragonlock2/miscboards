#include <ch32v00x.h>
#include "ticker.h"

/* private data */
static void (*ticker_cb)(void) = nullptr;

/* private helpers */
__attribute__((interrupt))
static void ticker_isr(void) {
    SysTick->SR = 0;
    if (ticker_cb) {
        ticker_cb();
    }
}

/* public functions */
void ticker_start(uint64_t period, void (*cb)(void)) {
    NVIC_SetVector(SysTicK_IRQn, ticker_isr);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = period - 1; // note ch32v003 is uint32_t
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
    ticker_cb = cb;
}
