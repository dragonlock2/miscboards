#include <ch32v00x.h>
#include <ch32v00x_exti.h>
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

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource3);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource4);
}

bool btn_read(btn c) {
    uint8_t status = 0xFF;
    switch (c) {
        case btn::up:       status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4); break;
        case btn::down:     status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3); break;
        case btn::dispense: status = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2); break;
        default: break;
    }
    return !static_cast<bool>(status);
}

void btn_sleep(void) {
    EXTI_InitTypeDef exti = {
        .EXTI_Line    = EXTI_Line2 | EXTI_Line3 | EXTI_Line4,
        .EXTI_Mode    = EXTI_Mode_Event,
        .EXTI_Trigger = EXTI_Trigger_Falling,
        .EXTI_LineCmd = ENABLE,
    };
    EXTI_Init(&exti);
    // handler in motor.cpp, but unused
}

void btn_wake(void) {
    EXTI_InitTypeDef exti = {
        .EXTI_Line    = EXTI_Line2 | EXTI_Line3 | EXTI_Line4,
        .EXTI_Mode    = EXTI_Mode_Event,
        .EXTI_Trigger = EXTI_Trigger_Falling,
        .EXTI_LineCmd = DISABLE,
    };
    EXTI_Init(&exti);
}
