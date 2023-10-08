#ifndef SYSTEM_CH32V00X_H
#define SYSTEM_CH32V00X_H

#include <stdint.h>

extern uint32_t SystemCoreClock;

void SetSysClockTo_24MHZ_HSI(void);
void SetSysClockTo_48MHZ_HSI(void);

static inline void NVIC_SetVector(IRQn_Type type, void (*cb)(void)) {
    extern void (*vectors[39])(void);
    vectors[type] = cb;
}

static inline void SysTick_Start(uint32_t period, void (*cb)(void)) {
    NVIC_SetVector(SysTicK_IRQn, cb);
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR   = 0;
    SysTick->CMP  = period - 1;
    SysTick->CNT  = 0;
    SysTick->CTLR = 0x0000000F;
}

static inline void UniqueID(uint32_t id[3]) {
    id[0] = *(uint32_t*) 0x1FFFF7F0;
    id[1] = *(uint32_t*) 0x1FFFF7EC;
    id[2] = *(uint32_t*) 0x1FFFF7E8;
}

#endif // SYSTEM_CH32V00X_H
