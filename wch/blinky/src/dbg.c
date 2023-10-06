#include "ch32v00x_gpio.h"
#include "ch32v00x_rcc.h"
#include "ch32v00x_usart.h"

__attribute__((constructor(101)))
static void clock_init(void) {
    SetSysClockTo_48MHZ_HSI();
}

__attribute__((constructor))
static void dbg_init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Pin   = GPIO_Pin_5,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure = {
        .USART_BaudRate            = 115200,
        .USART_WordLength          = USART_WordLength_8b,
        .USART_StopBits            = USART_StopBits_1,
        .USART_Parity              = USART_Parity_No,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode                = USART_Mode_Tx,
    };
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

__attribute__((used))
int _write(int fd, char *buf, int size) {
    (void) fd;
    for (int i = 0; i < size; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, *buf++);
    }
    return size;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <sys/stat.h>
__attribute__((used)) int _close(int fd) { return -1; }
__attribute__((used)) int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
__attribute__((used)) int _isatty(int file) { return 1; }
__attribute__((used)) int _lseek(int file, int ptr, int dir) { return 0; }
__attribute__((used)) int _read (int file, char * ptr, int len) { return 0; }
#pragma GCC diagnostic pop
