#define F_CPU 20000000L

#include <util/delay.h>
#include "mcc_generated_files/system/system.h"
#include "ws2812b.h"
#include "lsm303ah.h"
#include "app.h"

/* private defines */

/* private helpers */
static uint16_t app_read_vbatt() { // mV
    return ADC0_GetConversionResult() * 64 / 13; // * 2 * 2500mV / 1023 (close enough)
}

static uint8_t app_read_mic() { // unsigned raw
    return ADC1_GetConversionResult();
}

static void app_sleep() {
    // TODO prepare the things for sleeping (wakeup interrupts)
}

static void app_wakeup() {
    // TODO re-init things as needed
}

/* public functions */
void app_init() {
    // MCC Melody don't generate the enum...
    ADC0_StartConversion(ADC_MUXPOS_AIN8_gc); // vbatt_sense
    ADC1_StartConversion(ADC_MUXPOS_AIN3_gc); // mic
    
    reg_en_SetHigh();
    _delay_ms(100); // TODO what's the right amount
    
    ws2812b_write(5, 5, 5);
    
    lsm303ah_init();
    
    // TODO nrf24 w/ int
    // TODO write the full app (measure current!)
        // 5s timeout to high power sleep
        // 15s timeout to low power sleep
        // doing audio in real time will be a challenge bc gotta read other things too, need to make everything run in the background
        // need to re-init stuffs if wake from low power
        // add keep-on mode using button presses
        // add way to hit low power sleep using buttons alone
}

void app_loop() {
    _delay_ms(10);
}
