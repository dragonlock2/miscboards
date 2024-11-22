#include <iostream>
#include <chip.h>

const uint32_t OscRateIn    = 12000000;
const uint32_t RTCOscRateIn = 0;

__attribute__((constructor(101)))
static void dbg_init(void) {
    // setup 72MHz clock
    Chip_FMC_SetFLASHAccess(SYSCTL_FLASHTIM_72MHZ_CPU);
    Chip_SetupXtalClocking();
    SystemCoreClockUpdate();

    // setup other clocks
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    Chip_Clock_SetSysTickClockDiv(1);
    Chip_Clock_SetUARTBaseClockRate(Chip_Clock_GetMainClockRate(), false);
    Chip_SYSCTL_PeriphReset(RESET_IOCON);

    // setup USART0
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 14, IOCON_MODE_INACT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 15, IOCON_MODE_INACT);
    Chip_SWM_MovablePortPinAssign(SWM_UART0_RXD_I, 0, 14);
    Chip_SWM_MovablePortPinAssign(SWM_UART0_TXD_O, 0, 15);

    Chip_UART_Init(LPC_USART0);
	Chip_UART_ConfigData(LPC_USART0, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
	Chip_UART_SetBaud(LPC_USART0, 115200);
	Chip_UART_Enable(LPC_USART0);
	Chip_UART_TXEnable(LPC_USART0);
}

namespace __cxxabiv1 {
    std::terminate_handler __terminate_handler = std::abort;
}

extern "C" {

__attribute__((used))
int _write(int fd, char *ptr, int len) {
    (void) fd;
    Chip_UART_SendBlocking(LPC_USART0, ptr, len);
    return len;
}

#include <sys/stat.h>
__attribute__((used)) int _read(int, char*, int) { return -1; }
__attribute__((used)) int _open(const char*, int, ...) { return -1; }
__attribute__((used)) int _close(int) { return -1; }
__attribute__((used)) int _link(const char*, const char*) { return -1; }
__attribute__((used)) int _unlink(const char*) { return -1; }
__attribute__((used)) int _stat(const char*, struct stat*) { return -1; }
__attribute__((used)) int _fstat(int, struct stat*) { return -1; }
__attribute__((used)) int _isatty(int) { return -1; }
__attribute__((used)) int _lseek(int, int, int) { return -1; }
__attribute__((used)) int _kill(pid_t, int) { return -1; }
__attribute__((used)) pid_t _getpid(void) { return 0; }

}
