#include <cstddef>
#include <cstdio>
#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <ch32v20x.h>
#include <ch32v20x_gpio.h>
#include "fpga.h"

/* private helpers */
static void gpio_init(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = pin,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = mode,
    };
    GPIO_Init(port, &cfg);
}

static void spi_init(void) {
    gpio_init(GPIOA, GPIO_Pin_1, GPIO_Mode_Out_PP);      // mosi
    gpio_init(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING); // miso
    gpio_init(GPIOA, GPIO_Pin_0, GPIO_Mode_Out_PP);      // sck
    gpio_init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP);      // cs

    // mode 0 idle levels
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET); // sck
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);   // cs
}

static void spi_deinit(void) {
    gpio_init(GPIOA, GPIO_Pin_1, GPIO_Mode_IN_FLOATING); // mosi
    gpio_init(GPIOA, GPIO_Pin_3, GPIO_Mode_IN_FLOATING); // miso
    gpio_init(GPIOA, GPIO_Pin_0, GPIO_Mode_IN_FLOATING); // sck
    gpio_init(GPIOA, GPIO_Pin_2, GPIO_Mode_IN_FLOATING); // cs
}

template <std::size_t N>
static void spi_transceive(std::array<uint8_t, N>& data, bool release=true) {
    // mode 0, msb first
    taskENTER_CRITICAL();
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_RESET);
    for (uint8_t& out: data) {
        uint8_t in = 0;
        for (int i = 0; i < 8; i++) {
            GPIO_WriteBit(GPIOA, GPIO_Pin_1, static_cast<BitAction>(out & 0x80));
            out <<= 1;
            GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
            in <<= 1;
            in |= GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
            GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
        }
        out = in;
    }
    if (release) {
        taskEXIT_CRITICAL();
        GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);
    }
}

template <std::size_t N>
static void spi_write(const std::array<uint8_t, N>& data, bool release=true) {
    std::array<uint8_t, N> tmp = data;
    spi_transceive(tmp, release);
}

template <std::size_t N>
static void spi_read(std::array<uint8_t, N>& data, bool release=true) {
    data.fill(0xFF);
    spi_transceive(data, release);
}

static void flash_init(void) {
    // fpga puts flash to sleep, wake it up
    spi_write(std::array<uint8_t, 1> { 0xAB });
    for (int i = 0; i < 1000; i++) { asm ("nop"); } // >3us
}

/* public functions */
void fpga_init(void) {
    gpio_init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_OD);      // creset
    gpio_init(GPIOB, GPIO_Pin_1, GPIO_Mode_IN_FLOATING); // cdone

    gpio_init(GPIOA, GPIO_Pin_8, GPIO_Mode_AF_PP); // clk
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_HSE;

    fpga_on();
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

bool fpga_erase(uint32_t addr) {
    if (addr % 4096 != 0) { return false; }

    // unlock
    spi_write(std::array<uint8_t, 1> { 0x06 });

    // erase
    spi_write(std::array<uint8_t, 4> {
        0x20,
        static_cast<uint8_t>((addr >> 16) & 0xFF),
        static_cast<uint8_t>((addr >> 8)  & 0xFF),
        static_cast<uint8_t>((addr >> 0)  & 0xFF),
    });

    // wait
    std::array<uint8_t, 2> wait;
    do {
        wait = { 0x05, 0xFF };
        spi_transceive(wait);
    } while (wait[1] & 0x01);

    return true;
}

bool fpga_write(uint32_t addr, const std::array<uint8_t, 256> &data) {
    if (addr % 256 != 0) { return false; }

    // unlock
    spi_write(std::array<uint8_t, 1> { 0x06 });

    // write
    spi_write(std::array<uint8_t, 4> {
        0x02,
        static_cast<uint8_t>((addr >> 16) & 0xFF),
        static_cast<uint8_t>((addr >> 8)  & 0xFF),
        static_cast<uint8_t>((addr >> 0)  & 0xFF),
    }, false);
    spi_write(data);

    // wait
    std::array<uint8_t, 2> wait;
    do {
        wait = { 0x05, 0xFF };
        spi_transceive(wait);
    } while (wait[1] & 0x01);

    return true;
}

bool fpga_read(uint32_t addr, std::array<uint8_t, 256> &data) {
    spi_write(std::array<uint8_t, 4> {
        0x03,
        static_cast<uint8_t>((addr >> 16) & 0xFF),
        static_cast<uint8_t>((addr >> 8)  & 0xFF),
        static_cast<uint8_t>((addr >> 0)  & 0xFF),
    }, false);
    spi_read(data);
    return true;
}
