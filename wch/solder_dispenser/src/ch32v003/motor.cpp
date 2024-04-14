#include <ch32v00x.h>
#include <ch32v00x_exti.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_tim.h>
#include "motor.h"

static struct {
    bool dir;
    volatile int32_t ticks;
} data;

__attribute__((interrupt))
static void motor_isr(void) {
    if (EXTI_GetFlagStatus(EXTI_Line1)) {
        EXTI_ClearFlag(EXTI_Line1);
        if (data.dir) {
            data.ticks += 1;
        } else {
            data.ticks -= 1;
        }
    }
}

__attribute__((constructor))
static void motor_init(void) {
    // motor control
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

    // ripple counting encoder
    GPIO_InitTypeDef ripple_cfg = {
        .GPIO_Pin   = GPIO_Pin_1,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_IPD,
    };
    GPIO_Init(GPIOC, &ripple_cfg);

    EXTI_InitTypeDef exti = {
        .EXTI_Line    = EXTI_Line1,
        .EXTI_Mode    = EXTI_Mode_Interrupt,
        .EXTI_Trigger = EXTI_Trigger_Rising,
        .EXTI_LineCmd = ENABLE,
    };
    EXTI_Init(&exti);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
    NVIC_SetVector(EXTI7_0_IRQn, motor_isr);
    NVIC_EnableIRQ(EXTI7_0_IRQn);
}

void motor_write(bool dir, uint8_t duty) {
    TIM_SetCompare3(TIM1, duty);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, static_cast<BitAction>(!dir));
    if (data.dir != dir || duty == 0) {
        data.ticks = 0;
    }
    data.dir = dir;
}

int32_t motor_read(void) {
    return data.ticks;
}

void motor_sleep(void) {
    // allow SDI wake, interestingly also lowers current draw
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource1);
}

void motor_wake(void) {
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
}
