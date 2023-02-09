#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "flash.h"
#include "audio.h"

/* private data */
typedef struct {
    audio_ticker_F ticker;
} audio_data_S;
static audio_data_S audio_data;

/* private helpers */
ISR(TCA0_OVF_vect) {
    // 25kHz ISR
    TCA0.SINGLE.INTFLAGS = 0x01; // clear OVF

    // TODO test
    static uint8_t i = 0;
    TCA0.SINGLE.CMP2BUF = i++;

    audio_data.ticker();
}

/* public functions */
void audio_init(audio_ticker_F ticker) {
    flash_init();

    // setup TCA0
    TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV2_gc | 0x01; // ENABLE=1
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | 0x40; // CMP2EN=1
    TCA0.SINGLE.INTCTRL = 0x01; // OVF=1
    TCA0.SINGLE.PER     = F_CPU / 2 / PWM_FREQ - 1;
    
    // WO2 => PB5
    PORTB.DIRSET  = PIN5_bm;
    PORTMUX_CTRLC = PORTMUX_TCA02_bm;

    audio_data.ticker = ticker;
}
