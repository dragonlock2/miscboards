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

static inline uint16_t FlashSize(void) {
    return 16; // KiB
}

static inline void UniqueID(uint32_t id[3]) {
    id[0] = *(uint32_t*) 0x1FFFF7F0;
    id[1] = *(uint32_t*) 0x1FFFF7EC;
    id[2] = *(uint32_t*) 0x1FFFF7E8;
}

#endif // SYSTEM_CH32V00X_H
