#include <cstdio>
#include <chip.h>
#include "spi.h"

void spi_init(void) {
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8,  IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,  IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 11, IOCON_MODE_INACT);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MOSI_IO,     0, 8);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MISO_IO,     0, 5);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SCK_IO,      0, 18);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SSELSN_0_IO, 0, 11);

    SPI_CFG_T cfg = {
        .Mode      = SPI_MODE_MASTER,
        .ClockMode = SPI_CLOCK_MODE0,
        .DataOrder = SPI_DATA_MSB_FIRST,
        .SSELPol   = SPI_CFG_SPOL0_LO,
        .ClkDiv    = static_cast<uint16_t>(Chip_SPI_CalClkRateDivider(LPC_SPI0, 24000000)),
    };
    SPI_DELAY_CONFIG_T delay_cfg = {
        .PreDelay      = 0, // >17ns
        .PostDelay     = 0, // >17ns
        .FrameDelay    = 0,
        .TransferDelay = 1, // >40ns
    };
    Chip_SPI_Init(LPC_SPI0);
    Chip_SPI_SetConfig(LPC_SPI0, &cfg);
    Chip_SPI_DelayConfig(LPC_SPI0, &delay_cfg);
    Chip_SPI_Enable(LPC_SPI0);
}

void spi_transceive(uint8_t* tx, uint8_t* rx, size_t len) {
    // TODO use DMA
    SPI_DATA_SETUP_T setup = {
        .pTx      = tx,
        .TxCnt    = 0,
        .pRx      = rx,
        .RxCnt    = 0,
        .Length   = len,
        .ssel     = SPI_TXCTL_ASSERT_SSEL0,
        .DataSize = 8,
    };
    Chip_SPI_RWFrames_Blocking(LPC_SPI0, &setup);
}
