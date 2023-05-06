#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "audio.h"

/* private data */
struct {
    audio_ticker_F ticker;

    uint8_t r, g, b;
    uint8_t fast, slow;

    uint32_t boot;
    uint32_t hum;
    uint32_t off;
    uint32_t on;
    uint32_t swing[10];
    uint32_t clash[10];

    // TODO update
    bool new;
    uint32_t addr;
    uint32_t len;
} audio_data;

/* private helpers */
// low-level flash helpers
static inline void audio_write_byte_blocking(uint8_t b) {
    SPI0.DATA = b;
    while (!(SPI0.INTFLAGS & SPI_RXCIF_bm));
}

static inline void audio_start_read(uint32_t addr) {
    PORTC.DIRSET = PIN2_bm;
    audio_write_byte_blocking(0x03);
    audio_write_byte_blocking(addr >> 16);
    audio_write_byte_blocking(addr >> 8);
    audio_write_byte_blocking(addr >> 0);
    SPI0.DATA = 0xFF; // send initial dummy byte
}

static inline void audio_end_read() {
    while (!(SPI0.INTFLAGS & SPI_RXCIF_bm));
    PORTC.DIRCLR = PIN2_bm;
}

static inline uint8_t audio_read_byte() {
    uint8_t b = SPI0.DATA;
    SPI0.DATA = 0xFF; // dummy byte
    return b;
}

static inline uint8_t audio_read_byte_blocking() {
    while (!(SPI0.INTFLAGS & SPI_RXCIF_bm));
    return audio_read_byte();
}

// LightFS helpers
static inline uint32_t audio_fs_read_word() {
    uint32_t b = audio_read_byte_blocking();
    b = (b << 8) | audio_read_byte_blocking();
    b = (b << 8) | audio_read_byte_blocking();
    b = (b << 8) | audio_read_byte_blocking();
    return b;
}

ISR(TCA0_OVF_vect) {
    // 25kHz ISR
    TCA0.SINGLE.INTFLAGS = 0x01; // clear OVF

    // TODO add loop support
    if (audio_data.new) {
        audio_end_read();
        audio_start_read(audio_data.addr);
        audio_data.len = audio_fs_read_word();
        audio_data.new = false;
        TCA0.SINGLE.CMP1BUF = 128;
    } else if (audio_data.len) {
        TCA0.SINGLE.CMP1BUF = audio_read_byte();
        audio_data.len--;
    } else {
        audio_end_read();
        TCA0.SINGLE.CMP1BUF = 128;
    }
    audio_data.ticker();
}

/* public functions */
void audio_init(audio_ticker_F ticker) {
    PORTA.DIRSET = PIN1_bm | PIN3_bm;
    PORTC.DIRCLR = PIN2_bm; // open-drain CS
    PORTC.OUTCLR = PIN2_bm;

    SPI0.CTRLA   = SPI_PRESC_DIV4_gc | SPI_MASTER_bm | SPI_CLK2X_bm; // MSB first, master, 10MHz
    SPI0.CTRLB   = SPI_MODE_0_gc | SPI_SSD_bm; // no buffer, disable CS, MODE0
    SPI0.INTCTRL = 0x00; // no IRQs
    SPI0.CTRLA  |= SPI_ENABLE_bm;

    // read font 0
    audio_start_read(0x000000);
    if (audio_fs_read_word() != PWM_FREQ) {
        printf("sample frequency mismatch!\r\n");
    }
    uint32_t font_addr = audio_fs_read_word();
    audio_end_read();

    audio_start_read(font_addr);
    audio_data.r    = audio_read_byte_blocking();
    audio_data.g    = audio_read_byte_blocking();
    audio_data.b    = audio_read_byte_blocking();
    uint8_t s       = audio_read_byte_blocking();
    audio_data.fast = (s >> 0) & 0x0F;
    audio_data.slow = (s >> 4) & 0x0F;
    audio_data.boot = audio_fs_read_word();
    audio_data.hum  = audio_fs_read_word();
    audio_data.off  = audio_fs_read_word();
    audio_data.on   = audio_fs_read_word();
    for (int i = 0; i < 10; i++) {
        audio_data.swing[i] = audio_fs_read_word();
    }
    for (int i = 0; i < 10; i++) {
        audio_data.clash[i] = audio_fs_read_word();
    }
    audio_end_read();

    // setup TCA0 for WO1
    TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV2_gc | TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP1EN_bm;
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.PER     = F_CPU / 2 / PWM_FREQ - 1; // keep >= 256
    TCA0.SINGLE.CMP1BUF = 128;
    
    // WO2 => PB4
    PORTB.DIRSET  = PIN4_bm;
    PORTMUX.CTRLC = PORTMUX_TCA01_bm;

    // set PWM pin on STPIN250 to enable audio
    PORTB.DIRSET = PIN5_bm;
    PORTB.OUTSET = PIN5_bm;

    audio_data.ticker = ticker;

    // TODO update and remove
    audio_data.addr = audio_data.boot;
    audio_data.new  = true;
}
