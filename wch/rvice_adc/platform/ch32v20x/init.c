#include <FreeRTOS.h>
#include <stdio.h>
#include <ch32v20x.h>
#include <ch32v20x_gpio.h>
#include <ch32v20x_rcc.h>
#include <ch32v20x_usart.h>

__attribute__((constructor(101)))
void clock_init(void) {
    SetSysClockTo144_HSE();
    configASSERT(SystemCoreClock == configCPU_CLOCK_HZ);

    switch (SystemCoreClock) {
        case 48000000:  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1); break;
        case 96000000:  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div2); break;
        case 144000000: RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3); break;
        default: __disable_irq(); while (1); break;
    }

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,   ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,   ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,  ENABLE);
}

__attribute__((constructor(102)))
void dbg_init(void) {
    GPIO_InitTypeDef tx = {
        .GPIO_Pin   = GPIO_Pin_9,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOA, &tx);

    GPIO_InitTypeDef rx = {
        .GPIO_Pin   = GPIO_Pin_10,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    };
    GPIO_Init(GPIOA, &rx);

    USART_InitTypeDef cfg = {
        .USART_BaudRate            = 115200,
        .USART_WordLength          = USART_WordLength_8b,
        .USART_StopBits            = USART_StopBits_1,
        .USART_Parity              = USART_Parity_No,
        .USART_Mode                = USART_Mode_Tx | USART_Mode_Rx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    };
    USART_Init(USART1, &cfg);
    USART_Cmd(USART1, ENABLE);
}

__attribute__((used))
int _write(int fd, char *buf, int len) {
    if (fd == stdout->_file) {
        for (int i = 0; i < len; i++) {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
            USART_SendData(USART1, *buf++);
        }
        return len;
    }
    return -1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <sys/stat.h>
__attribute__((used)) int _read (int fd, char *ptr, int len) { return -1; }
__attribute__((used)) int _close(int fd) { return -1; }
__attribute__((used)) int _fstat(int fd, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
__attribute__((used)) int _isatty(int fd) { return 1; }
__attribute__((used)) int _lseek(int fd, int ptr, int dir) { return 0; }
__attribute__((used)) int _kill(pid_t pid, int sig) { return -1; }
__attribute__((used)) pid_t _getpid(void) { return 0; }
#pragma GCC diagnostic pop
