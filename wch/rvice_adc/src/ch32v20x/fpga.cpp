#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ch32v20x.h>
#include <ch32v20x_gpio.h>
#include "fpga.h"

/* private defines */
#define GPIO_INIT(port, pin, mode)          \
    {                                       \
        GPIO_InitTypeDef cfg = {            \
            .GPIO_Pin   = pin,              \
            .GPIO_Speed = GPIO_Speed_50MHz, \
            .GPIO_Mode  = mode,             \
        };                                  \
        GPIO_Init(port, &cfg);              \
    }

/* private helpers */
static void spi_init(void) {
    GPIO_INIT(GPIOA, GPIO_Pin_1, GPIO_Mode_Out_PP);      // mosi
    GPIO_INIT(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING); // miso
    GPIO_INIT(GPIOA, GPIO_Pin_0, GPIO_Mode_Out_PP);      // sck
    GPIO_INIT(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP);      // cs

    // mode 0 idle levels
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET); // sck
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);   // cs
}

static void spi_deinit(void) {
    GPIO_INIT(GPIOA, GPIO_Pin_1, GPIO_Mode_IN_FLOATING); // mosi
    GPIO_INIT(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING); // miso
    GPIO_INIT(GPIOA, GPIO_Pin_0, GPIO_Mode_IN_FLOATING); // sck
    GPIO_INIT(GPIOA, GPIO_Pin_2, GPIO_Mode_IN_FLOATING); // cs
}

static void spi_release(void) {
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);
}

static void spi_transceive(uint8_t *data, size_t len, bool release=true) {
    // mode 0, msb first
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET);
    for (size_t i = 0; i < len; i++) {
        uint8_t out = data[i], in = 0;
        for (int j = 0; j < 8; j++) {
            GPIO_WriteBit(GPIOA, GPIO_Pin_1, static_cast<BitAction>(out & 0x80));
            out <<= 1;
            GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
            in <<= 1;
            in |= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
            GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
        }
        data[i] = in;
    }
    if (release) {
        spi_release();
    }
}

static inline void spi_write(const uint8_t *data, size_t len, bool release=true) {
    uint8_t tmp[len];
    memcpy(tmp, data, len);
    spi_transceive(tmp, len, release);
}

static inline void spi_read(uint8_t *data, size_t len, bool release=true) {
    memset(data, 0xFF, len);
    spi_transceive(data, len, release);
}

static void flash_init(void) {
    // fpga puts flash to sleep, wake it up
    const uint8_t wake = 0xAB;
    spi_write(&wake, 1);
    for (int i = 0; i < 500; i++) { asm ("nop"); } // >3us
}

/* public functions */
__attribute__((constructor))
static void fpga_init(void) {
    GPIO_INIT(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_OD);      // creset
    GPIO_INIT(GPIOB, GPIO_Pin_1, GPIO_Mode_IN_FLOATING); // cdone
    fpga_off();

    GPIO_INIT(GPIOA, GPIO_Pin_8, GPIO_Mode_AF_PP); // clk
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_HSE;
}

void fpga_off(void) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
    spi_init();
    flash_init();
}

void fpga_on(void) {
    spi_deinit();
    for (int i = 0; i < 30; i++) { asm ("nop"); } // >200ns
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
}

bool fpga_booted(void) {
    return static_cast<bool>(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1));
}
