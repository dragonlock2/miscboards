#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include "pwr.h"

__attribute__((constructor))
static void pwr_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_0,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &cfg);
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
}

void pwr_sleep(void) {
    // TODO disable peripherals
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
    // TODO actually sleep
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    // TODO enable peripherals
}
