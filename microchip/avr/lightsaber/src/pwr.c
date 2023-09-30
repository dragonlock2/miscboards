#include <avr/io.h>
#include <util/delay.h>
#include "pwr.h"

/* private defines */
#define PWR_PORT (PORTB)
#define PWR_PIN  (PIN3_bm)

#define VBAT_PORT (PORTA)
#define VBAT_PIN  (PIN7_bm)
#define VBAT_CTRL (PORTA.PIN7CTRL)
#define VBAT_CHAN (ADC_MUXPOS_AIN7_gc)

/* public functions */
void pwr_init() {
    PWR_PORT.DIRSET = PWR_PIN;

    VREF.CTRLA    = VREF_ADC0REFSEL_2V5_gc;
    ADC0.CTRLB    = ADC_SAMPNUM_ACC1_gc;
    ADC0.CTRLC    = ADC_SAMPCAP_bm | ADC_REFSEL_INTREF_gc | ADC_PRESC_DIV16_gc; // 20MHz / 16 < 1.5MHz
    ADC0.CTRLD    = ADC_INITDLY_DLY256_gc | ADC_ASDV_ASVOFF_gc | 0x00; // SAMPDLY=0
    ADC0.CTRLE    = ADC_WINCM_NONE_gc;
    ADC0.SAMPCTRL = 31;
    ADC0.MUXPOS   = VBAT_CHAN;
    ADC0.CALIB    = ADC_DUTYCYC_DUTY25_gc;
    ADC0.CTRLA    = ADC_ENABLE_bm;

    pwr_wake();
}

void pwr_sleep() {
    PWR_PORT.OUTCLR = PWR_PIN;
    VBAT_CTRL = PORT_PULLUPEN_bm;
}

void pwr_wake() {
    VBAT_CTRL = 0x00;
    PWR_PORT.OUTSET = PWR_PIN;
    _delay_ms(10);
}

uint16_t pwr_vbat() {
    ADC0.COMMAND = ADC_STCONV_bm;
    while (ADC0.COMMAND & ADC_STCONV_bm);
    uint16_t res = ADC0.RES;
    return res * 49 / 10; // approx. res / 1023 * 2500 * 2
}
