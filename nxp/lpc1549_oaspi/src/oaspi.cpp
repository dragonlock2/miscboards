#include <cstdio>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <chip.h>
#include "spi.h"
#include "oaspi.h"

/* private data */
static struct {
    StaticSemaphore_t mdio_lock_buf;
    SemaphoreHandle_t mdio_lock;
} data;

/* private helpers */
static bool oaspi_parity(uint8_t hdr[4]) {
    uint32_t val = *reinterpret_cast<uint32_t*>(hdr); // endian-independent
    val = val ^ (val >> 1);
    val = val ^ (val >> 2);
    val = val ^ (val >> 4);
    val = val ^ (val >> 8);
    val = val ^ (val >> 16);
    return !(val & 1);
}

static bool oaspi_control_check(uint8_t rx[16]) {
    if ((rx[4] & 0x40) || oaspi_parity(&rx[4])) { // HDRB or parity error
        return false;
    }
    if (((rx[8]  | rx[12]) != 0xFF) ||
        ((rx[9]  | rx[13]) != 0xFF) ||
        ((rx[10] | rx[14]) != 0xFF) ||
        ((rx[11] | rx[15]) != 0xFF)) { // protection error
        return false;
    }
    return true;
}

static uint16_t oaspi_mdio_cmd(uint32_t cmd) {
    uint32_t ret;
    while (true) {
        oaspi_reg_write(oaspi_mms::STANDARD, 0x20, cmd);
        while (((ret = oaspi_reg_read(oaspi_mms::STANDARD, 0x20)) & 0x80000000) == 0);
        if (!(ret & 0x40000000)) { // turnaround error
            return ret & 0xFFFF;
        }
    }
}

/* public functions */
void oaspi_init(void) {
    data.mdio_lock = xSemaphoreCreateMutexStatic(&data.mdio_lock_buf);

    // hardware reset
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    // TODO MAC init

    // PHY led setting
    uint16_t tmp;
    tmp = oaspi_mdio_c45_read(0x1E, 0x8C56);
    oaspi_mdio_c45_write(0x1E, 0x8C56, tmp & ~0x000E); // enable LED1
    tmp = oaspi_mdio_c45_read(0x1E, 0x8C83);
    oaspi_mdio_c45_write(0x1E, 0x8C83, tmp | 0x0005); // active-high
    tmp = oaspi_mdio_c45_read(0x1E, 0x8C82);
    oaspi_mdio_c45_write(0x1E, 0x8C82, (tmp & ~(0x1f1f)) | 0x0304); // LED0: act, LED1: link

    // PHY turn on
    oaspi_mdio_c45_write(0x1E, 0x8812, 0x0000);
    while (oaspi_mdio_c45_read(0x1E, 0x8818) & 0x0002);
}

void oaspi_reg_write(oaspi_mms mms, uint16_t reg, uint32_t val) {
    while (true) {
        uint8_t tx[16], rx[16];
        tx[0]  = 0x20 | static_cast<uint8_t>(mms);
        tx[1]  = (reg >> 8) & 0xFF;
        tx[2]  = reg & 0xFF;
        tx[3]  = 0x00;
        tx[3] |= oaspi_parity(tx);
        tx[4]  = (val >> 24) & 0xFF;
        tx[5]  = (val >> 16) & 0xFF;
        tx[6]  = (val >> 8)  & 0xFF;
        tx[7]  = (val >> 0)  & 0xFF;
        tx[8]  = ~tx[4];
        tx[9]  = ~tx[5];
        tx[10] = ~tx[6];
        tx[11] = ~tx[7];
        spi_transceive(tx, rx, 16);
        if (oaspi_control_check(rx)) {
            break;
        }
    }
}

uint32_t oaspi_reg_read(oaspi_mms mms, uint16_t reg) {
    while (true) {
        uint8_t tx[16], rx[16];
        tx[0] = 0x00 | static_cast<uint8_t>(mms);
        tx[1] = (reg >> 8) & 0xFF;
        tx[2] = reg & 0xFF;
        tx[3] = 0;
        tx[3] |= oaspi_parity(tx);
        spi_transceive(tx, rx, 16);
        if (oaspi_control_check(rx)) {
            return (rx[8] << 24) | (rx[9] << 16) | (rx[10] << 8) | (rx[11] << 0);
        }
    }
}

void oaspi_mdio_write(uint8_t reg, uint16_t val) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x14200000;
    cmd |= (reg & 0x1F) << 16;
    cmd |= val;
    oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
}

uint16_t oaspi_mdio_read(uint8_t reg) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x18200000;
    cmd |= (reg & 0x1F) << 16;
    uint16_t ret = oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
    return ret;
}

void oaspi_mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x00200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= reg;
    oaspi_mdio_cmd(cmd);
    cmd  = 0x04200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= val;
    oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
}

uint16_t oaspi_mdio_c45_read(uint8_t devad, uint16_t reg) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x00200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= reg;
    oaspi_mdio_cmd(cmd);
    cmd  = 0x0c200000;
    cmd |= (devad & 0x1F) << 16;
    uint16_t ret = oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
    return ret;
}
