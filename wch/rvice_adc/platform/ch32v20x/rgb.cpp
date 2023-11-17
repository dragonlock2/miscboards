#include <ch32v20x.h>
#include <ch32v20x_gpio.h>
#include "rgb.h"

/* public functions */
void rgb_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = 0,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };

    cfg.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &cfg);
    cfg.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOC, &cfg);
    cfg.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOC, &cfg);

    rgb_write(0, 0, 0);
}

void rgb_write(bool r, bool g, bool b) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, static_cast<BitAction>(!r));
    GPIO_WriteBit(GPIOC, GPIO_Pin_14, static_cast<BitAction>(!b));
    GPIO_WriteBit(GPIOC, GPIO_Pin_15, static_cast<BitAction>(!g));
}
