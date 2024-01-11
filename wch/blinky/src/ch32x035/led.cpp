#include <stdbool.h>
#include <ch32x035.h>
#include <ch32x035_gpio.h>
#include <ch32x035_rcc.h>
#include "led.h"

#define LED_PORT (GPIOA)
#define LED_PIN  (1 << 7)
#define LED_CLK  (RCC_APB2Periph_GPIOA)

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
    GPIO_WriteBit(LED_PORT, LED_PIN, (BitAction) !i);
}
