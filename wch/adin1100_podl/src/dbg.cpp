#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_rcc.h>
#include <ch32v00x_usart.h>

__attribute__((constructor(101)))
static void clock_init(void) {
    SetSysClockTo_48MHZ_HSI();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

__attribute__((constructor))
static void dbg_init(void) {
    GPIO_InitTypeDef dbg = {
        .GPIO_Pin   = GPIO_Pin_5,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode  = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOD, &dbg);

    USART_InitTypeDef usart = {
        .USART_BaudRate            = 115200,
        .USART_WordLength          = USART_WordLength_8b,
        .USART_StopBits            = USART_StopBits_1,
        .USART_Parity              = USART_Parity_No,
        .USART_Mode                = USART_Mode_Tx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    };
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

extern "C" {

__attribute__((used))
int _write(int fd, char* buf, int size) {
    (void) fd;
    for (int i = 0; i < size; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, *buf++);
    }
    return size;
}

#include <sys/stat.h>
__attribute__((used)) int _close(int) { return -1; }
__attribute__((used)) int _fstat(int, struct stat*) { return -1; }
__attribute__((used)) int _isatty(int) { return -1; }
__attribute__((used)) int _lseek(int, int, int) { return -1; }
__attribute__((used)) int _read (int, char*, int) { return -1; }
__attribute__((used)) int _kill(pid_t, int) { return -1; }
__attribute__((used)) pid_t _getpid(void) { return 0; }

}
