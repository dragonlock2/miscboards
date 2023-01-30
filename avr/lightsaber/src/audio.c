#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
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

    audio_data.ticker();
}

/* public functions */
void audio_init(audio_ticker_F ticker) {
    // setup TCA0
    TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV2_gc | 0x01; // ENABLE=1
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.INTCTRL = 0x01; // OVF=1
    TCA0.SINGLE.PER     = F_CPU / 2 / PWM_FREQ - 1;

    audio_data.ticker = ticker;
}
