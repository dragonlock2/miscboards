#include <cstdbool>
#include <tusb.h>
#include <ch32v20x.h>
#include <ch32v20x_dma.h>
#include <ch32v20x_exti.h>
#include <ch32v20x_gpio.h>
#include <ch32v20x_spi.h>
#include "fpga.h"
#include "audio.h"

/* private data */
static constexpr size_t SAMPLES_PER_MS = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;

static struct {
    TaskHandle_t audio_handle;
    bool ping;
    int16_t dummy;
    int16_t buffer[2][SAMPLES_PER_MS];
} data;

/* private helpers */
static void gpio_init(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode) {
    GPIO_InitTypeDef cfg = {
        .GPIO_Pin   = pin,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = mode,
    };
    GPIO_Init(port, &cfg);
}

static void dma_start(void) {
    // init dma
    DMA_InitTypeDef tx_cfg = {
        .DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&SPI1->DATAR),
        .DMA_MemoryBaseAddr     = reinterpret_cast<uint32_t>(&data.dummy),
        .DMA_DIR                = DMA_DIR_PeripheralDST,
        .DMA_BufferSize         = SAMPLES_PER_MS,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Disable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
        .DMA_Mode               = DMA_Mode_Normal,
        .DMA_Priority           = DMA_Priority_VeryHigh,
        .DMA_M2M                = DMA_M2M_Disable,
    };
    DMA_DeInit(DMA1_Channel3);
    DMA_Init(DMA1_Channel3, &tx_cfg);

    DMA_InitTypeDef rx_cfg = {
        .DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&SPI1->DATAR),
        .DMA_MemoryBaseAddr     = reinterpret_cast<uint32_t>(&data.buffer[data.ping][0]),
        .DMA_DIR                = DMA_DIR_PeripheralSRC,
        .DMA_BufferSize         = SAMPLES_PER_MS,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Enable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
        .DMA_Mode               = DMA_Mode_Normal,
        .DMA_Priority           = DMA_Priority_VeryHigh,
        .DMA_M2M                = DMA_M2M_Disable,
    };
    DMA_DeInit(DMA1_Channel2);
    DMA_Init(DMA1_Channel2, &rx_cfg);

    // avoid race condition, start rx first
    data.dummy++;
    DMA_Cmd(DMA1_Channel2, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

static void gpio_handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    EXTI_ClearFlag(EXTI_Line4);
    vTaskNotifyGiveIndexedFromISR(data.audio_handle, 0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* public functions */
void audio_task(void* args) {
    (void) args;
    while (true) {
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY); // wait for signal for enough samples
        dma_start();
        tud_audio_write(reinterpret_cast<uint8_t*>(data.buffer[!data.ping]), sizeof(data.buffer[0]));
        data.ping = !data.ping;
    }
    vTaskDelete(NULL);
}

void audio_init(void) {
    // init gpio
    gpio_init(GPIOA, GPIO_Pin_7, GPIO_Mode_AF_PP);       // mosi1
    gpio_init(GPIOA, GPIO_Pin_6, GPIO_Mode_IN_FLOATING); // miso1
    gpio_init(GPIOA, GPIO_Pin_5, GPIO_Mode_AF_PP);       // sck1
    gpio_init(GPIOA, GPIO_Pin_4, GPIO_Mode_IPD);         // cs1

    // init spi
    SPI_InitTypeDef spi_cfg = {
        .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
        .SPI_Mode              = SPI_Mode_Master,
        .SPI_DataSize          = SPI_DataSize_16b,
        .SPI_CPOL              = SPI_CPOL_Low,
        .SPI_CPHA              = SPI_CPHA_1Edge,
        .SPI_NSS               = SPI_NSS_Soft,
        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16, // 9MHz
        .SPI_FirstBit          = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial     = 0,
    };
    SPI_Init(SPI1, &spi_cfg);
	SPI_Cmd(SPI1, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);

    // init gpio interrupt
    EXTI_InitTypeDef ext_cfg = {
        .EXTI_Line    = EXTI_Line4,
        .EXTI_Mode    = EXTI_Mode_Interrupt,
        .EXTI_Trigger = EXTI_Trigger_Rising,
        .EXTI_LineCmd = ENABLE,
    };
    EXTI_Init(&ext_cfg);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4);
    NVIC_SetVector(EXTI4_IRQn, gpio_handler);
    NVIC_EnableIRQ(EXTI4_IRQn);

    xTaskCreate(audio_task, "audio_task", configMINIMAL_STACK_SIZE, NULL,
        configMAX_PRIORITIES - 2, &data.audio_handle);
}
