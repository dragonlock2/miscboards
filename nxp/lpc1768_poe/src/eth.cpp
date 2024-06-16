#include <cstdio>
#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <chip.h>
#include "eth.h"

/* private constants */
static const size_t NUM_RX_DESC = 4;
static const size_t NUM_TX_DESC = 4;

static const uint8_t ETH_ADDR[6] = {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00}; // TESLA. a.s. :D

/* private data */
static uint8_t tx_buff[NUM_TX_DESC][ENET_ETH_MAX_FLEN] __attribute__((section(".ram1")));
static uint8_t rx_buff[NUM_RX_DESC][ENET_ETH_MAX_FLEN] __attribute__((section(".ram2")));

static struct {
    ENET_TXSTAT_T tx_stat[NUM_TX_DESC] __attribute__((aligned(4)));
    ENET_TXDESC_T tx_desc[NUM_TX_DESC] __attribute__((aligned(4)));
    ENET_RXSTAT_T rx_stat[NUM_RX_DESC] __attribute__((aligned(8)));
    ENET_RXDESC_T rx_desc[NUM_RX_DESC] __attribute__((aligned(4)));
    TaskHandle_t eth_phy_handle;
    bool link_up;
} data;

/* private helpers */
static void eth_phy_isr(void) {
    Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT2, 1 << 2);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(data.eth_phy_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void eth_isr(void) {
    uint32_t status = Chip_ENET_GetIntStatus(LPC_ETHERNET);
    Chip_ENET_ClearIntStatus(LPC_ETHERNET, status);
}

static void eth_phy_task(void*) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        eth_mdio_read(0x1E); // clear interrupt
        eth_mdio_read(0x01); // clear status
        uint16_t status = eth_mdio_read(0x01);
        if (status & (1 << 2)) {
            status = eth_mdio_read(0x00);
            printf("link up - ");
            if (status & (1 << 13)) {
                printf("100Mbps, ");
                Chip_ENET_Set100Mbps(LPC_ETHERNET);
            } else {
                printf("10Mbps, ");
                Chip_ENET_Set10Mbps(LPC_ETHERNET);
            }
            if (status & (1 << 8)) {
                printf("full-duplex\r\n");
                Chip_ENET_SetFullDuplex(LPC_ETHERNET);
            } else {
                printf("half-duplex\r\n");
                Chip_ENET_SetHalfDuplex(LPC_ETHERNET);
            }
            data.link_up = true;
        } else {
            printf("link down\r\n");
            data.link_up = false;
        }
    }
    vTaskDelete(NULL);
}

/* public functions */
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

    // setup MDIO
    Chip_ENET_Init(LPC_ETHERNET, true);
    Chip_ENET_SetupMII(LPC_ETHERNET, Chip_ENET_FindMIIDiv(LPC_ETHERNET, 2500000), 0b00000);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 2, 2);
    Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT2, 1 << 2);
    NVIC_SetVector(EINT3_IRQn, (uint32_t) eth_phy_isr);
    NVIC_SetPriority(EINT3_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(EINT3_IRQn);

    // setup PHY
    eth_mdio_write(0x00, 0x8000);
    while (eth_mdio_read(0x00) & 0x8000) { // software reset
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    eth_mdio_write(0x1F, 0x0007); // page 7
    eth_mdio_write(0x13, 0x2000); // enable link change interrupt, LED0 > ACT_ALL, LED1 > LINK100
    eth_mdio_write(0x1F, 0x0000); // page 0
    xTaskCreate(eth_phy_task, "eth_phy_task", configMINIMAL_STACK_SIZE, NULL, 1, &data.eth_phy_handle);

    // setup ETH
    for (size_t i = 0; i < NUM_TX_DESC; i++) {
        data.tx_stat[i].StatusInfo = 0xFFFFFFFF;
        data.tx_desc[i].Packet     = (uint32_t) tx_buff[i];
        data.tx_desc[i].Control    = ENET_TCTRL_SIZE(ENET_ETH_MAX_FLEN) | ENET_TCTRL_INT;
    }
    for (size_t i = 0; i < NUM_RX_DESC; i++) {
        data.rx_stat[i].StatusInfo    = 0xFFFFFFFF;
        data.rx_stat[i].StatusHashCRC = 0xFFFFFFFF;
        data.rx_desc[i].Packet        = (uint32_t) rx_buff[i];
        data.rx_desc[i].Control       = ENET_RCTRL_SIZE(ENET_ETH_MAX_FLEN) | ENET_RCTRL_INT;
    }
    Chip_ENET_InitTxDescriptors(LPC_ETHERNET, data.tx_desc, data.tx_stat, NUM_TX_DESC);
    Chip_ENET_InitRxDescriptors(LPC_ETHERNET, data.rx_desc, data.rx_stat, NUM_RX_DESC);
    Chip_ENET_SetADDR(LPC_ETHERNET, ETH_ADDR);
    // Chip_ENET_EnableRXFilter(LPC_ETHERNET, ENET_RXFILTERCTRL_ABE | ENET_RXFILTERCTRL_APE);
    Chip_ENET_EnableInt(LPC_ETHERNET, ENET_INT_RXOVERRUN  | ENET_INT_RXERROR | ENET_INT_RXDONE |
                                      ENET_INT_TXUNDERRUN | ENET_INT_TXERROR | ENET_INT_TXDONE);
    Chip_ENET_TXEnable(LPC_ETHERNET);
	Chip_ENET_RXEnable(LPC_ETHERNET);
    NVIC_SetVector(ETHERNET_IRQn, (uint32_t) eth_isr);
    NVIC_SetPriority(ETHERNET_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(ETHERNET_IRQn);

    // fix for errata https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf
    eth_write(NULL, 0);
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

bool eth_link_up(void) {
    return data.link_up;
}

void eth_write(uint8_t* buffer, size_t len) {
    // TODO zero-copy
    while (Chip_ENET_IsTxFull(LPC_ETHERNET));
    uint16_t idx = Chip_ENET_GetTXProduceIndex(LPC_ETHERNET);
    memcpy(tx_buff[idx], buffer, len);
    data.tx_desc[idx].Control = ENET_TCTRL_SIZE(len) | ENET_TCTRL_LAST | ENET_TCTRL_INT;
    Chip_ENET_IncTXProduceIndex(LPC_ETHERNET);
}

bool eth_read(uint8_t*& buffer, size_t& len) {
    while (!Chip_ENET_IsRxEmpty(LPC_ETHERNET)) {
        uint16_t idx = Chip_ENET_GetRXConsumeIndex(LPC_ETHERNET);
        if (data.rx_stat[idx].StatusInfo & (ENET_RINFO_CRC_ERR | ENET_RINFO_SYM_ERR | ENET_RINFO_ALIGN_ERR | ENET_RINFO_LEN_ERR)) {
            // drop packet
            Chip_ENET_IncRXConsumeIndex(LPC_ETHERNET);
        } else {
            // app needs to call eth_read_done()
            buffer = rx_buff[idx];
            len    = ENET_RINFO_SIZE(data.rx_stat[idx].StatusInfo) - 4; // remove FCS
            return true;
        }
    }
    return false;
}

void eth_read_done(void) {
    Chip_ENET_IncRXConsumeIndex(LPC_ETHERNET);
}
