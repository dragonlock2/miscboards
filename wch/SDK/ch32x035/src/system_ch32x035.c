#include <ch32x035.h>

/* private data */
uint32_t SystemCoreClock = 8000000;

/* public functions */
void SetSysClockTo_48MHZ_HSI(void) {
    // slower flash clock (>24MHz)
    FLASH->ACTLR = (FLASH->ACTLR & ~FLASH_ACTLR_LATENCY) | FLASH_ACTLR_LATENCY_2;

    // AHB prescaler
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_HPRE) | RCC_HPRE_DIV1;

    SystemCoreClock = 48000000;
}

void Delay_Init(void) {}

__attribute__((naked))
void Delay_Us(uint32_t us) {
    // approximately correct at 48MHz w/ 2 wait states
    asm volatile (
        "mv t0, %0              \n"
        "slli t0, t0, 2         \n"
        "beqz t0, Delay_Us_done \n"
    "Delay_Us_loop:"
        "addi t0, t0, -1        \n"
        "bnez t0, Delay_Us_loop \n"
    "Delay_Us_done:"
        "ret                    \n"
        : : "r" (us) : "t0"
    );
}

