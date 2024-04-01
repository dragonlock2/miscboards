#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_tim.h>
#include "rgb.h"

__attribute__((constructor))
static void rgb_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOD, &cfg);
    cfg.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOC, &cfg);
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef tim_base = { // ~1.9kHz
        .TIM_Prescaler         = 100 - 1,
        .TIM_CounterMode       = TIM_CounterMode_Up,
        .TIM_Period            = 256 - 1,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };
    TIM_TimeBaseInit(TIM2, &tim_base);

    TIM_OCInitTypeDef tim_oc = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Enable,
        .TIM_Pulse        = 0,
        .TIM_OCPolarity   = TIM_OCPolarity_Low,
        .TIM_OCNPolarity  = TIM_OCNPolarity_Low,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    };
    TIM_OC2Init(TIM2, &tim_oc);
    TIM_OC3Init(TIM2, &tim_oc);
    TIM_OC4Init(TIM2, &tim_oc);

    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_CtrlPWMOutputs(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void rgb_write(uint8_t r, uint8_t g, uint8_t b) {
    TIM_SetCompare2(TIM2, r);
    TIM_SetCompare3(TIM2, g);
    TIM_SetCompare4(TIM2, b);
}
