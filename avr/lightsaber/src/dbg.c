#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dbg.h"

#define BAUD_RATE (115200L)

/* private helpers */
static int uart_putc(char c, FILE *f) {
    USART0.TXDATAL = c;
    while (!(USART0.STATUS & USART_TXCIF_bm));
    USART0.STATUS |= USART_TXCIF_bm;
    return 0;
}

/* public functions */
void dbg_init() {
    USART0.BAUD  = 4L * F_CPU / BAUD_RATE; // 64*fclk_per/16/fbaud
    USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc |
                   USART_PMODE_DISABLED_gc |
                   USART_SBMODE_1BIT_gc |
                   USART_CHSIZE_8BIT_gc; // 8N1 UART
    PORTB.DIRSET = PIN2_bm; // TX (PB2) as output
    USART0.CTRLB = USART_TXEN_bm | USART_RXMODE_NORMAL_gc; // enable TX

    static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);
    stdout = &uart_stdout;
}
