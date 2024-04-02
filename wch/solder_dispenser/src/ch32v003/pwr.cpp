#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_pwr.h>
#include <ch32v00x_rcc.h>
#include "btn.h"
#include "motor.h"
#include "pwr.h"
#include "vbat.h"

__attribute__((constructor))
static void pwr_init(void) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = GPIO_Pin_0,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &cfg);
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);

    // unused pins can't float
    GPIO_InitTypeDef unused_cfg = {
        .GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_2,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_IPU,
    };
    GPIO_Init(GPIOA, &unused_cfg);
    unused_cfg.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOC, &unused_cfg);
    unused_cfg.GPIO_Pin = GPIO_Pin_1; // SWIO too
    GPIO_Init(GPIOD, &unused_cfg);
}

void pwr_sleep(void) {
    // disable peripherals
    vbat_sleep();
    motor_sleep();
    btn_sleep();
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);

    // deep sleep (note need power cycle after flashing)
    PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);
    extern void clock_init(void);
    clock_init();

    // enable peripherals
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    btn_wake();
    motor_wake();
    vbat_wake();
}
