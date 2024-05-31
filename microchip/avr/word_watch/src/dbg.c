#include <stdio.h>
#include <avr/io.h>

static int dbg_putc(char c, FILE *f) {
    while (!(USART0.STATUS & USART_DREIF_bm));
    USART0.TXDATAL = c;
    return 0;
}

void dbg_init(void) {
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00; // F_CPU => 20MHz

    USART0.BAUD    = (64 * F_CPU) / 16 / 115200;
    USART0.CTRLC   = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc;
    PORTA.DIRSET   = PIN1_bm;
    PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;
    USART0.CTRLB   = USART_TXEN_bm | USART_RXMODE_NORMAL_gc;

    static FILE uart_stdout = FDEV_SETUP_STREAM(dbg_putc, NULL, _FDEV_SETUP_WRITE);
    stdout = &uart_stdout;
}
