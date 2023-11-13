#include <ch32v20x.h>
#include <ch32v20x_flash.h>
#include <ch32v20x_rcc.h>

/* private data */
uint32_t SystemCoreClock = 8000000;

/* public functions */
void SetSysClockTo8_HSI(void) {
    // enable HSI
    RCC->CTLR |= RCC_HSION;
    while ((RCC->CTLR & RCC_HSIRDY) == 0);

    // switch to HSI
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_HSI;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_HSI);

    // AHB, APB1, APB2 prescalers
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_HPRE)  | RCC_HPRE_DIV1;
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_PPRE1) | RCC_PPRE1_DIV1;
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_PPRE2) | RCC_PPRE2_DIV1;

    // faster flash clock (<60MHz)
    FLASH_Unlock_Fast();
    FLASH->CTLR |= 1 << 25; // SCKMOD=1
    FLASH_Lock_Fast();

    SystemCoreClock = 8000000;
}

void SetSysClockTo144_HSI(void) {
    SetSysClockTo8_HSI();

    // slower flash clock (>=60MHz)
    FLASH_Unlock_Fast();
    FLASH->CTLR &= ~(1 << 25); // SCKMOD=0
    FLASH_Lock_Fast();

    // configure PLL
    RCC->CTLR  &= ~RCC_PLLON;
    RCC->CFGR0 &= ~RCC_PLLSRC;
    RCC->CFGR0  = (RCC->CFGR0 & ~RCC_PLLMULL) | RCC_PLLMULL18;
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    // enable PLL
    RCC->CTLR |= RCC_PLLON;
    while ((RCC->CTLR & RCC_PLLRDY) == 0);

    // switch to PLL
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_PLL);

    SystemCoreClock = 144000000;
}

void SetSysClockTo144_HSE(void) {
    SetSysClockTo8_HSI();

    // slower flash clock (>=60MHz)
    FLASH_Unlock_Fast();
    FLASH->CTLR &= ~(1 << 25); // SCKMOD=0
    FLASH_Lock_Fast();

    // enable HSE
    RCC->CTLR |= RCC_HSEON;
    while ((RCC->CTLR & RCC_HSERDY) == 0);

    // configure PLL
    RCC->CTLR  &= ~RCC_PLLON;
    RCC->CFGR0 &= ~(RCC_PLLSRC | RCC_PLLXTPRE);
    RCC->CFGR0 |= RCC_PLLSRC;
    RCC->CFGR0  = (RCC->CFGR0 & ~RCC_PLLMULL) | RCC_PLLMULL18; // assume 8MHz crystal

    // enable PLL
    RCC->CTLR |= RCC_PLLON;
    while ((RCC->CTLR & RCC_PLLRDY) == 0);

    // switch to PLL
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_PLL);

    SystemCoreClock = 144000000;
}
