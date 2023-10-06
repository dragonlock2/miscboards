#ifndef SYSTEM_CH32V00X_H
#define SYSTEM_CH32V00X_H

extern uint32_t SystemCoreClock;

void SetSysClockTo_48MHZ_HSI(void);

static inline void NVIC_SetVector(IRQn_Type type, void (*fun)(void)) {
    extern void (*vectors[39])(void);
    vectors[type] = fun;
}

#endif // SYSTEM_CH32V00X_H
