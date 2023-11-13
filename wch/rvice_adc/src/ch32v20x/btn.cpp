#include <ch32v20x.h>
#include <ch32v20x_gpio.h>
#include "btn.h"

/* public functions */
__attribute__((constructor))
static void btn_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_9,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_IPU,
    };
    GPIO_Init(GPIOB, &cfg);
}

bool btn_read(void) {
    return !static_cast<bool>(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9));
}
