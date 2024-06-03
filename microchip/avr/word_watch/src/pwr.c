#include <avr/io.h>
#include <avr/sleep.h>
#include "btn.h"
#include "dbg.h"
#include "matrix.h"
#include "pwr.h"

void pwr_init(void) {
    VREF.CTRLA   = VREF_ADC0REFSEL_2V5_gc;
    ADC0.CTRLA   = 0;
    ADC0.CTRLB   = ADC_SAMPNUM_ACC1_gc;
    ADC0.CTRLC   = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV8_gc;
    ADC0.MUXPOS  = ADC_MUXPOS_INTREF_gc;
    ADC0.CALIB   = ADC_DUTYCYC_DUTY50_gc;
    ADC0.CTRLA  |= ADC_ENABLE_bm;
}

uint16_t pwr_vbat(void) {
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0.COMMAND & ADC_STCONV_bm);
    uint32_t raw = ADC0.RES;
    return (2500L * 1023L) / raw;
}

void pwr_sleep(void) {
    dbg_sleep();
    matrix_sleep();
    btn_sleep();

    // can shorten time to sleep from RTC wake for marginal current improvement
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();

    btn_wake();
    matrix_wake();
}
