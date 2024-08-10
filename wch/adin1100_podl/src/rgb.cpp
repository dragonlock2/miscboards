#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include "rgb.h"

__attribute__((constructor))
static void rgb_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &cfg);
    rgb_write(0, 0, 0);
}

void rgb_write(bool r, bool g, bool b) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_4, static_cast<BitAction>(!r));
    GPIO_WriteBit(GPIOC, GPIO_Pin_5, static_cast<BitAction>(!b));
    GPIO_WriteBit(GPIOC, GPIO_Pin_7, static_cast<BitAction>(!g));
}
