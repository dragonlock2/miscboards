#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "eth.h"

void eth_init(void) {
    // setup IO
    Chip_IOCON_EnableOD(LPC_IOCON, 4, 28);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 4, 28);
    Chip_GPIO_SetPinState(LPC_GPIO, 4, 28, true); // no reset

    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 0,  IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 1,  IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 4,  IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 8,  IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9,  IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 10, IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 14, IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 15, IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 16, IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 17, IOCON_MODE_INACT | IOCON_FUNC1);

    // setup ETH
    Chip_ENET_Init(LPC_ETHERNET, true);
    Chip_ENET_SetupMII(LPC_ETHERNET, Chip_ENET_FindMIIDiv(LPC_ETHERNET, 2500000), 0b00000);

    // setup PHY
    eth_mdio_write(0x00, 0x8000);
    while (eth_mdio_read(0x00) & 0x8000) { // software reset
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    eth_mdio_write(0x1F, 0x0007);
    eth_mdio_write(0x13, 0x0000); // LED0 > ACT_ALL, LED1 > LINK100
}

void eth_mdio_write(uint8_t reg, uint16_t val) {
    Chip_ENET_StartMIIWrite(LPC_ETHERNET, reg, val);
    while (Chip_ENET_IsMIIBusy(LPC_ETHERNET));
}

uint16_t eth_mdio_read(uint8_t reg) {
    Chip_ENET_StartMIIRead(LPC_ETHERNET, reg);
    while (Chip_ENET_IsMIIBusy(LPC_ETHERNET));
    return Chip_ENET_ReadMIIData(LPC_ETHERNET);
}
