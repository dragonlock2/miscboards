#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include "mdio.h"

// inspired by linux/drivers/net/mdio/mdio-bitbang.c

/* private defines */
enum mdio_op {
    MDIO_WRITE     = 0x0001,
    MDIO_READ      = 0x0002, // plus addr for clause 45
    MDIO_C45       = 0x8000,
    MDIO_C45_ADDR  = 0x8000,
    MDIO_C45_WRITE = 0x8001,
    MDIO_C45_READ  = 0x8003,
};

/* hardware layer */
__attribute__((constructor))
static void mdio_init(void) {
    GPIO_InitTypeDef mdio = {
        .GPIO_Pin   = GPIO_Pin_0,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_OD,
    };
    GPIO_Init(GPIOC, &mdio);
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);

    GPIO_InitTypeDef mdc = {
        .GPIO_Pin   = GPIO_Pin_1,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &mdc);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);

    GPIO_InitTypeDef rst = {
        .GPIO_Pin   = GPIO_Pin_2,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_Out_PP,
    };
    GPIO_Init(GPIOC, &rst);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_RESET);
    for (int i = 0; i < 200000; i++) {
        asm volatile ("nop");
    }
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, Bit_SET);
    for (int i = 0; i < 1000000; i++) {
        asm volatile ("nop");
    }
}

static void mdio_delay(void) {
    for (int i = 0; i < 100; i++) {
        asm volatile ("nop");
    }
}

static void mdio_send_bit(bool b) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, static_cast<BitAction>(b));
    mdio_delay();
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
    mdio_delay();
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
}

static bool mdio_get_bit(void) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
    mdio_delay();
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
    mdio_delay();
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
    return static_cast<bool>(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0));
}

/* private helpers */
static void mdio_send_num(uint16_t val, int num) {
    for (int i = num - 1; i >= 0; i--) {
        mdio_send_bit((val >> i) & 1);
    }
}

static uint16_t mdio_get_num(int num) {
    uint16_t ret = 0;
    for (int i = num - 1; i >= 0; i--) {
        ret = (ret << 1) | mdio_get_bit();
    }
    return ret;
}

static void mdio_cmd(uint16_t op, uint8_t phy, uint8_t reg) {
    for (int i = 0; i < 32; i++) {
        mdio_send_bit(1);
    }
    mdio_send_bit(0);
    mdio_send_bit(!(op & MDIO_C45));
    mdio_send_bit((op >> 1) & 1);
    mdio_send_bit((op >> 0) & 1);
    mdio_send_num(phy, 5);
    mdio_send_num(reg, 5);
}

static void mdio_cmd_addr(uint8_t phy, uint8_t dev_addr, uint16_t reg) {
    mdio_cmd(MDIO_C45_ADDR, phy, dev_addr);
    mdio_send_bit(1);
    mdio_send_bit(0);
    mdio_send_num(reg, 16);
    mdio_get_bit(); // dummy
}

static void mdio_write_common(uint16_t val) {
    mdio_send_bit(1);
    mdio_send_bit(0);
    mdio_send_num(val, 16);
    mdio_get_bit(); // dummy
}

static uint16_t mdio_read_common(void) {
    if (mdio_get_bit() != 0) {
        for (int i = 0; i < 32; i++) {
            mdio_get_bit();
        }
        return 0xFFFF;
    }
    uint16_t ret = mdio_get_num(16);
    mdio_get_bit(); // dummy
    return ret;
}

/* public functions */
void mdio_write(uint8_t phy, uint8_t reg, uint16_t val) {
    mdio_cmd(MDIO_WRITE, phy, reg);
    mdio_write_common(val);
}

uint16_t mdio_read(uint8_t phy, uint8_t reg) {
    mdio_cmd(MDIO_READ, phy, reg);
    return mdio_read_common();
}

void mdio_write_c45(uint8_t phy, uint8_t devad, uint16_t reg, uint16_t val) {
    mdio_cmd_addr(phy, devad, reg);
    mdio_cmd(MDIO_C45_WRITE, phy, devad);
    mdio_write_common(val);
}

uint16_t mdio_read_c45(uint8_t phy, uint8_t devad, uint16_t reg) {
    mdio_cmd_addr(phy, devad, reg);
    mdio_cmd(MDIO_C45_READ, phy, devad);
    return mdio_read_common();
}
