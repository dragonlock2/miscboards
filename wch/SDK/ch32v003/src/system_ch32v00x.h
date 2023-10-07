#ifndef SYSTEM_CH32V00X_H
#define SYSTEM_CH32V00X_H

extern uint32_t SystemCoreClock;

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

#endif // SYSTEM_CH32V00X_H
