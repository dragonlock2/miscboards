#include <ch32v00x.h>
#include <ch32v00x_rcc.h>

/* private data */
uint32_t SystemCoreClock = 8000000; // default

/* public functions */
void SetSysClockTo_24MHZ_HSI(void) {
    // enable HSI
    RCC->CTLR |= RCC_HSION;
    while ((RCC->CTLR & RCC_HSIRDY) == 0);

    // switch to HSI
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_HSI;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_HSI);

    // AHB prescaler
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_HPRE) | RCC_HPRE_DIV1;

    // set flash 0 wait state (0-24MHz)
    FLASH->ACTLR = (FLASH->ACTLR & ~FLASH_ACTLR_LATENCY) | FLASH_ACTLR_LATENCY_0;

    SystemCoreClock = 24000000;
}

void SetSysClockTo_48MHZ_HSI(void) {
    SetSysClockTo_24MHZ_HSI();

    // set flash 1 wait state (24-48MHz)
    FLASH->ACTLR = (FLASH->ACTLR & ~FLASH_ACTLR_LATENCY) | FLASH_ACTLR_LATENCY_1;

    // configure PLL
    RCC->CTLR  &= ~RCC_PLLON;
    RCC->CFGR0 &= ~RCC_PLLSRC;

    // enable PLL
    RCC->CTLR |= RCC_PLLON;
    while ((RCC->CTLR & RCC_PLLRDY) == 0);

    // switch to PLL
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_PLL);

    SystemCoreClock = 48000000;
}
