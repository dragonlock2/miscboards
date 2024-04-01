#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include "btn.h"

__attribute__((constructor))
static void btn_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_IPU,
    };
    GPIO_Init(GPIOD, &cfg);
}

bool btn_read(btn c) {
    uint8_t status = 0xFF;
    switch (c) {
        case btn::up:       status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2); break;
        case btn::down:     status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3); break;
        case btn::dispense: status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4); break;
    }
    return !static_cast<bool>(status);
}
