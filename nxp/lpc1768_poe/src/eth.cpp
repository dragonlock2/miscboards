#include <array>
#include <cstdio>
#include <cstring>
#include <random>
#include <FreeRTOS.h>
#include <FreeRTOS_IP.h>
#include <task.h>
#include <chip.h>
#include "eth.h"

/* private constants */
#define NUM_TX_DESC  (2) // few queued packets
#define NUM_RX_DESC  (4)
#define RX_INT_GROUP (ENET_INT_RXOVERRUN  | ENET_INT_RXERROR | ENET_INT_RXDONE)
#define DNS_NAME     "lpc1768_poe"

/* private data */
__attribute__((section(".ram2")))
static std::array<std::array<uint8_t, ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING>, ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS> ucBuffers;

static struct {
    std::array<ENET_TXSTAT_T, NUM_TX_DESC> tx_stat __attribute__((aligned(4)));
    std::array<ENET_TXDESC_T, NUM_TX_DESC> tx_desc __attribute__((aligned(4)));
    std::array<ENET_RXSTAT_T, NUM_RX_DESC> rx_stat __attribute__((aligned(8)));
    std::array<ENET_RXDESC_T, NUM_RX_DESC> rx_desc __attribute__((aligned(4)));
    std::array<NetworkBufferDescriptor_t*, NUM_RX_DESC>  tx_netdesc;
    std::array<NetworkBufferDescriptor_t*, NUM_RX_DESC>  rx_netdesc;
    TaskHandle_t phy_handle, rx_handle;
    bool link_up;

    NetworkInterface_t net_if;
    NetworkEndPoint_t net_ep;
    std::minstd_rand rand_eng;
    std::uniform_int_distribution<uint32_t> rand_dist;
} data;

/* private helpers */
static void eth_phy_isr(void) {
    Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT2, 1 << 2);
    BaseType_t woke = pdFALSE;
    vTaskNotifyGiveFromISR(data.phy_handle, &woke);
    portYIELD_FROM_ISR(woke);
}

static void eth_isr(void) {
    uint32_t status = Chip_ENET_GetIntStatus(LPC_ETHERNET);
    Chip_ENET_ClearIntStatus(LPC_ETHERNET, status);
    BaseType_t woke = pdFALSE;
    if (status & RX_INT_GROUP) {
        vTaskNotifyGiveFromISR(data.rx_handle, &woke);
    }
    portYIELD_FROM_ISR(woke);
}

static void eth_phy_write(uint8_t reg, uint16_t val) {
    Chip_ENET_StartMIIWrite(LPC_ETHERNET, reg, val);
    while (Chip_ENET_IsMIIBusy(LPC_ETHERNET));
}

static uint16_t eth_phy_read(uint8_t reg) {
    Chip_ENET_StartMIIRead(LPC_ETHERNET, reg);
    while (Chip_ENET_IsMIIBusy(LPC_ETHERNET));
    return Chip_ENET_ReadMIIData(LPC_ETHERNET);
}

static void eth_phy_task(void*) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        eth_phy_read(0x1E); // clear interrupt
        eth_phy_read(0x01); // clear status
        uint16_t status = eth_phy_read(0x01);
        if (status & (1 << 2)) {
            status = eth_phy_read(0x00);
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
            FreeRTOS_NetworkDown(&data.net_if);
            data.link_up = true;
        } else {
            printf("link down\r\n");
            data.link_up = false;
        }
    }
    vTaskDelete(NULL);
}

static void eth_rx_task(void*) {
    IPStackEvent_t rx_event;
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (!Chip_ENET_IsRxEmpty(LPC_ETHERNET)) {
            uint16_t idx = Chip_ENET_GetRXConsumeIndex(LPC_ETHERNET);
            if (data.rx_stat[idx].StatusInfo & (ENET_RINFO_CRC_ERR | ENET_RINFO_SYM_ERR | ENET_RINFO_ALIGN_ERR | ENET_RINFO_LEN_ERR)) {
                Chip_ENET_IncRXConsumeIndex(LPC_ETHERNET); // drop packet
            } else {
                // get new buffer
                NetworkBufferDescriptor_t* new_netdesc = NULL;
                while (new_netdesc == NULL) {
                    new_netdesc = pxGetNetworkBufferWithDescriptor(ipTOTAL_ETHERNET_FRAME_SIZE, portMAX_DELAY);
                }

                // send to stack
                data.rx_netdesc[idx]->xDataLength = ENET_RINFO_SIZE(data.rx_stat[idx].StatusInfo) - 4; // remove FCS
                data.rx_netdesc[idx]->pxInterface = &data.net_if;
                data.rx_netdesc[idx]->pxEndPoint = FreeRTOS_MatchingEndpoint(&data.net_if, data.rx_netdesc[idx]->pucEthernetBuffer);
                if (data.rx_netdesc[idx]->pxEndPoint != NULL) {
                    rx_event.eEventType = eNetworkRxEvent;
                    rx_event.pvData = static_cast<void*>(data.rx_netdesc[idx]);
                    if (xSendEventStructToIPTask(&rx_event, 0) != pdFALSE) {
                        iptraceNETWORK_INTERFACE_RECEIVE();
                    } else {
                        vReleaseNetworkBufferAndDescriptor(data.rx_netdesc[idx]);
                        iptraceETHERNET_RX_EVENT_LOST();
                    }
                } else {
                    vReleaseNetworkBufferAndDescriptor(data.rx_netdesc[idx]);
                }

                // use new buffer
                data.rx_netdesc[idx] = new_netdesc;
                data.rx_desc[idx].Packet = reinterpret_cast<uint32_t>(new_netdesc->pucEthernetBuffer); // same buffer size
                Chip_ENET_IncRXConsumeIndex(LPC_ETHERNET);
            }
        }
    }
    vTaskDelete(NULL);
}

static BaseType_t eth_pf_init(struct xNetworkInterface* pxDescriptor) {
    if (data.phy_handle != NULL) {
        return pdPASS; // already initialized
    }

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
    NVIC_SetVector(EINT3_IRQn, reinterpret_cast<uint32_t>(eth_phy_isr));
    NVIC_SetPriority(EINT3_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(EINT3_IRQn);

    // setup PHY
    eth_phy_write(0x00, 0x8000);
    while (eth_phy_read(0x00) & 0x8000) { // software reset
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    eth_phy_write(0x1F, 0x0007); // page 7
    eth_phy_write(0x13, 0x2000); // enable link change interrupt, LED0 > ACT_ALL, LED1 > LINK100
    eth_phy_write(0x1F, 0x0000); // page 0
    xTaskCreate(eth_phy_task, "eth_phy_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &data.phy_handle);

    // setup ETH
    for (size_t i = 0; i < data.tx_desc.size(); i++) {
        data.tx_netdesc[i]         = NULL;
        data.tx_stat[i].StatusInfo = 0xFFFFFFFF;
        data.tx_desc[i].Packet     = static_cast<uint32_t>(NULL); // allocated later
        data.tx_desc[i].Control    = ENET_TCTRL_SIZE(0) | ENET_TCTRL_LAST | ENET_TCTRL_INT;
    }
    for (size_t i = 0; i < data.rx_desc.size(); i++) {
        data.rx_netdesc[i] = pxGetNetworkBufferWithDescriptor(ipTOTAL_ETHERNET_FRAME_SIZE, 0);
        configASSERT(data.rx_netdesc[i] != NULL);

        data.rx_stat[i].StatusInfo    = 0xFFFFFFFF;
        data.rx_stat[i].StatusHashCRC = 0xFFFFFFFF;
        data.rx_desc[i].Packet        = reinterpret_cast<uint32_t>(data.rx_netdesc[i]->pucEthernetBuffer);
        data.rx_desc[i].Control       = ENET_RCTRL_SIZE(ipTOTAL_ETHERNET_FRAME_SIZE) | ENET_RCTRL_INT;
    }
    Chip_ENET_InitTxDescriptors(LPC_ETHERNET, data.tx_desc.data(), data.tx_stat.data(), data.tx_desc.size());
    Chip_ENET_InitRxDescriptors(LPC_ETHERNET, data.rx_desc.data(), data.rx_stat.data(), data.rx_desc.size());
    Chip_ENET_SetADDR(LPC_ETHERNET, FreeRTOS_FirstEndPoint(pxDescriptor)->xMACAddress.ucBytes);
    Chip_ENET_EnableRXFilter(LPC_ETHERNET, ENET_RXFILTERCTRL_ABE | ENET_RXFILTERCTRL_AME | ENET_RXFILTERCTRL_APE);
    Chip_ENET_EnableInt(LPC_ETHERNET, RX_INT_GROUP); // only RX interrupts needed
    Chip_ENET_TXEnable(LPC_ETHERNET);
    Chip_ENET_RXEnable(LPC_ETHERNET);
    NVIC_SetVector(ETHERNET_IRQn, reinterpret_cast<uint32_t>(eth_isr));
    NVIC_SetPriority(ETHERNET_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(ETHERNET_IRQn);
    xTaskCreate(eth_rx_task, "eth_rx_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &data.rx_handle);

    // fix for errata https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf
    Chip_ENET_IncTXProduceIndex(LPC_ETHERNET);

    return pdPASS;
}

static BaseType_t eth_pf_output(struct xNetworkInterface* pxDescriptor,
                                NetworkBufferDescriptor_t* const pxNetworkBuffer, BaseType_t xReleaseAfterSend) {
    configASSERT(pxDescriptor == &data.net_if);
    while (Chip_ENET_IsTxFull(LPC_ETHERNET));
    uint16_t idx = Chip_ENET_GetTXProduceIndex(LPC_ETHERNET);
    if (data.tx_netdesc[idx] != NULL) {
        vReleaseNetworkBufferAndDescriptor(data.tx_netdesc[idx]); // wastes a bit of RAM, but simpler
        data.tx_netdesc[idx] = NULL;
    }
    data.tx_desc[idx].Packet  = reinterpret_cast<uint32_t>(pxNetworkBuffer->pucEthernetBuffer);
    data.tx_desc[idx].Control = ENET_TCTRL_SIZE(pxNetworkBuffer->xDataLength) | ENET_TCTRL_LAST | ENET_TCTRL_INT;
    Chip_ENET_IncTXProduceIndex(LPC_ETHERNET);
    if (xReleaseAfterSend != pdFALSE) {
        data.tx_netdesc[idx] = pxNetworkBuffer;
    }
    return pdFAIL;
}

static BaseType_t eth_pf_status(struct xNetworkInterface* pxDescriptor) {
    (void) pxDescriptor;
    return data.link_up ? pdTRUE : pdFALSE;
}

/* public functions */
void eth_init(void) {
    // setup RNG
    ADC_CLOCK_SETUP_T adc_setup;
    uint32_t seed = 0;
    uint16_t val;
    Chip_ADC_Init(LPC_ADC, &adc_setup);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 25, IOCON_MODE_INACT | IOCON_FUNC1); // floating pin
    Chip_ADC_EnableChannel(LPC_ADC, ADC_CH2, ENABLE);
    Chip_ADC_SetBurstCmd(LPC_ADC, DISABLE);
    for (int i = 0; i < 128; i++) {
        Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
        while (Chip_ADC_ReadStatus(LPC_ADC, ADC_CH2, ADC_DR_DONE_STAT) != SET);
        Chip_ADC_ReadValue(LPC_ADC, ADC_CH2, &val);
        seed += val;
    }
    data.rand_eng.seed(seed);

    // setup FreeRTOS IP
    data.net_if.pcName             = "eth0";
    data.net_if.pvArgument         = NULL;
    data.net_if.pfInitialise       = eth_pf_init;
    data.net_if.pfOutput           = eth_pf_output;
    data.net_if.pfGetPhyLinkStatus = eth_pf_status;
    FreeRTOS_AddNetworkInterface(&data.net_if);
    
    const std::array<uint8_t, 6> ucMACAddress       {0x00, 0x50, 0xC2, 0x4B, 0x20, 0x00}; // TESLA. a.s. :D
    const std::array<uint8_t, 4> ucIPAddress        {192, 168, 69, 42};
    const std::array<uint8_t, 4> ucNetMask          {255, 255, 255, 0};
    const std::array<uint8_t, 4> ucGatewayAddress   {192, 168, 69, 1};
    const std::array<uint8_t, 4> ucDNSServerAddress {8, 8, 8, 8};
    FreeRTOS_FillEndPoint(&data.net_if, &data.net_ep,
        ucIPAddress.data(), ucNetMask.data(), ucGatewayAddress.data(), ucDNSServerAddress.data(), ucMACAddress.data());
    if (ipconfigUSE_DHCP != 0) {
        data.net_ep.bits.bWantDHCP = pdTRUE;
    }

    FreeRTOS_IPInit_Multi();

    // TODO IPv6 support
}

extern "C" {

void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t xDescriptors[ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS]) {
    for (int i = 0; i < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; i++) {
        xDescriptors[i].pucEthernetBuffer = &ucBuffers[i][ipBUFFER_PADDING];
        *reinterpret_cast<uint32_t*>(&ucBuffers[i][0]) = reinterpret_cast<uint32_t>(&xDescriptors[i]); // TODO not needed soon?
    }
}

const char* pcApplicationHostnameHook(void) {
    return DNS_NAME;
}

BaseType_t xApplicationDNSQueryHook_Multi(struct xNetworkEndPoint* pxEndPoint, const char* pcName) {
    (void) pxEndPoint;
    if ((strncmp(DNS_NAME, pcName, 32) == 0) || (strncmp(DNS_NAME ".local", pcName, 32) == 0)) {
        return pdTRUE;
    }
    return pdFALSE;
}

BaseType_t xApplicationDNSQueryHook(const char* pcName) { // needed by mongoose, backwards compatibility
    return xApplicationDNSQueryHook_Multi(&data.net_ep, pcName);
}

BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber) {
    *pulNumber = data.rand_dist(data.rand_eng);
    return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort,
                                            uint32_t ulDestinationAddress, uint16_t usDestinationPort) {
    (void) ulSourceAddress;
    (void) usSourcePort;
    (void) ulDestinationAddress;
    (void) usDestinationPort;
    uint32_t r = 0;
    xApplicationGetRandomNumber(&r);
    return r;
}

}
