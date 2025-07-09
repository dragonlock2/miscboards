#include <cstdio>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <chip.h>
#include "spi.h"

/* private data */
static struct {
    SPI *dev;
    StaticSemaphore_t lock_buffer;
    SemaphoreHandle_t lock;
    TaskHandle_t waiter;
} data;

/* private helpers */
static void spi_dma_handler(void) {
    Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_SPI0_RX);
    Chip_SPI_ClearStatus(LPC_SPI0, SPI_STAT_CLR_SSA | SPI_STAT_CLR_SSD | SPI_STAT_FORCE_EOT); // end transfer
    BaseType_t woke = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(data.waiter, configNOTIF_SPI, &woke);
    portYIELD_FROM_ISR(woke);
}

/* public functions */
SPI::SPI(void) {
    configASSERT(data.dev == nullptr);
    data.dev = this;

    data.lock = xSemaphoreCreateMutexStatic(&data.lock_buffer);
    configASSERT(data.lock);

#ifdef CONFIG_ADIN1110
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8,  IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5,  IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 11, IOCON_MODE_INACT);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MOSI_IO,     0, 8);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MISO_IO,     0, 5);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SCK_IO,      0, 18);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SSELSN_0_IO, 0, 11);
#elifdef CONFIG_NCN26010
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 9,  IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 7,  IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8,  IOCON_MODE_INACT);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MOSI_IO,     0, 9);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_MISO_IO,     0, 7);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SCK_IO,      0, 10);
    Chip_SWM_MovablePortPinAssign(SWM_SPI0_SSELSN_0_IO, 0, 8);
#endif

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

    NVIC_SetVector(DMA_IRQn, reinterpret_cast<uint32_t>(spi_dma_handler));
    NVIC_SetPriority(DMA_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(DMA_IRQn);

    Chip_DMA_Init(LPC_DMA);
    Chip_DMA_Enable(LPC_DMA);
    Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));
    Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_SPI0_TX);
    Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_SPI0_RX);
    Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_SPI0_TX, DMA_CFG_PERIPHREQEN | DMA_CFG_CHPRIORITY(0));
    Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_SPI0_RX, DMA_CFG_PERIPHREQEN | DMA_CFG_CHPRIORITY(0));
    Chip_DMA_EnableIntChannel(LPC_DMA, DMAREQ_SPI0_RX);
}

SPI::~SPI() {
    configASSERT(false); // not supporting for now
}

void SPI::transceive(uint8_t *tx, uint8_t *rx, size_t len) {
    xSemaphoreTake(data.lock, portMAX_DELAY);
    data.waiter = xTaskGetCurrentTaskHandle();
    DMA_CHDESC_T tx_desc = {
        .xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SWTRIG | DMA_XFERCFG_WIDTH_8 |
                   DMA_XFERCFG_SRCINC_1 | DMA_XFERCFG_DSTINC_0 | DMA_XFERCFG_XFERCOUNT(len),
        .source  = DMA_ADDR(tx + len - 1),
        .dest    = DMA_ADDR(&LPC_SPI0->TXDAT),
        .next    = DMA_ADDR(0),
    };
    DMA_CHDESC_T rx_desc = {
        .xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SWTRIG | DMA_XFERCFG_SETINTA | DMA_XFERCFG_WIDTH_8 |
                   DMA_XFERCFG_SRCINC_0 | DMA_XFERCFG_DSTINC_1 | DMA_XFERCFG_XFERCOUNT(len),
        .source  = DMA_ADDR(&LPC_SPI0->RXDAT),
        .dest    = DMA_ADDR(rx + len - 1),
        .next    = DMA_ADDR(0),
    };
    Chip_SPI_SetControlInfo(LPC_SPI0, 8, SPI_TXCTL_ASSERT_SSEL0 | SPI_TXCTL_EOF);
    Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_SPI0_TX, &tx_desc);
    Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_SPI0_RX, &rx_desc);
    Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_SPI0_RX, rx_desc.xfercfg);
    Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_SPI0_TX, tx_desc.xfercfg); // start transfer
    ulTaskNotifyTakeIndexed(configNOTIF_SPI, true, portMAX_DELAY);
    xSemaphoreGive(data.lock);
}
