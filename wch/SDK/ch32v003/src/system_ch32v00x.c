#include "ch32v00x_rcc.h"
#include "system_ch32v00x.h"

uint32_t SystemCoreClock = 8000000; // default

void SetSysClockTo_48MHZ_HSI(void) {
    // TODO understand and rewrite
    // 8MHz -> 24MHz (divider) -> 48MHz w/ PLL? set all clocks?

    RCC->CFGR0 &= (uint32_t)0xFCFF0000;
    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFFFEFFFF;
    RCC->INTR = 0x009F0000;

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    GPIOD->CFGLR&=(~0xF0);
    GPIOD->CFGLR|=0x80;
    GPIOD->BSHR =0x2;

    /* Flash 0 wait state */
    FLASH->ACTLR &= (uint32_t)((uint32_t)~FLASH_ACTLR_LATENCY);
    FLASH->ACTLR |= (uint32_t)FLASH_ACTLR_LATENCY_1;

    /* HCLK = SYSCLK = APB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

    /* PLL configuration: PLLCLK = HSI * 2 = 48 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Mul2);

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0);
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;    
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);

    SystemCoreClock = 48000000;
}
