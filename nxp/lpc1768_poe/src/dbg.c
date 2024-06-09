#include <sys/stat.h>
#include <chip.h>

const uint32_t OscRateIn    = 12000000;
const uint32_t RTCOscRateIn = 0;

__attribute__((constructor(101)))
static void dbg_init(void) {
    // setup 96MHz clock
    Chip_SYSCTL_SetFLASHAccess(FLASHTIM_100MHZ_CPU);
    Chip_SetupXtalClocking();
    SystemCoreClockUpdate();

    // setup UART0
    Chip_IOCON_Init(LPC_IOCON);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 2, IOCON_MODE_INACT | IOCON_FUNC1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 3, IOCON_MODE_INACT | IOCON_FUNC1);

    Chip_UART_Init(LPC_UART0);
	Chip_UART_SetBaud(LPC_UART0, 115200);
	Chip_UART_ConfigData(LPC_UART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);
	Chip_UART_TXEnable(LPC_UART0);
}

__attribute__((used))
int _write(int fd, char *ptr, int len) {
    (void) fd;
    Chip_UART_SendBlocking(LPC_UART0, ptr, len);
    return len;
}

__attribute__((used))
int _read(int fd, char *ptr, int len) {
    (void) fd;
    while (!(Chip_UART_ReadLineStatus(LPC_UART0) & UART_LSR_RDR)); // wait until char available
    return Chip_UART_Read(LPC_UART0, ptr, len);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
__attribute__((used)) int _close(int fd) { return -1; }
__attribute__((used)) int _fstat(int fd, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
__attribute__((used)) int _isatty(int fd) { return 1; }
__attribute__((used)) int _lseek(int fd, int ptr, int dir) { return 0; }
#pragma GCC diagnostic pop
