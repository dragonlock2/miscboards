#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_tim.h>
#include "motor.h"

__attribute__((constructor))
static void motor_init(void) {
    GPIO_InitTypeDef dir_cfg = {
        .GPIO_Pin   = GPIO_Pin_2,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &dir_cfg);

    GPIO_InitTypeDef pwm_cfg = {
        .GPIO_Pin   = GPIO_Pin_3,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOC, &pwm_cfg);

    TIM_TimeBaseInitTypeDef tim_base = { // ~23.4kHz
        .TIM_Prescaler         = 8 - 1,
        .TIM_CounterMode       = TIM_CounterMode_Up,
        .TIM_Period            = 256 - 1,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };
    TIM_TimeBaseInit(TIM1, &tim_base);

    TIM_OCInitTypeDef tim_oc = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Enable,
        .TIM_Pulse        = 0,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCNPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    };
    TIM_OC3Init(TIM1, &tim_oc);

    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

void motor_write(bool dir, uint8_t duty) {
    TIM_SetCompare3(TIM1, duty);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, static_cast<BitAction>(dir));
}
