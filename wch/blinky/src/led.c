#include <stdbool.h>
#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_rcc.h>
#include "led.h"

#define LED_PORT (GPIOD)
#define LED_PIN  (1 << 0)
#define LED_CLK  (RCC_APB2Periph_GPIOD)

/* private helpers */
__attribute__((constructor))
static void led_init(void) {
    RCC_APB2PeriphClockCmd(LED_CLK, ENABLE);

    GPIO_InitTypeDef led = {
        .GPIO_Pin   = LED_PIN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(LED_PORT, &led);
}

/* public functions */
void led_toggle(void) {
    bool i = GPIO_ReadOutputDataBit(LED_PORT, LED_PIN);
    GPIO_WriteBit(LED_PORT, LED_PIN, !i);
}
