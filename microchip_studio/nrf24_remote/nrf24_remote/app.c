#define F_CPU 20000000L

#include <util/delay.h>
#include "mcc_generated_files/system/system.h"
#include "ws2812b.h"
#include "app.h"

/* private defines */

/* private helpers */
static uint16_t app_read_vbatt() { // mV
    return ADC0_GetConversionResult() * 64 / 13; // * 2 * 2500mV / 1023 (close enough)
}

static uint8_t app_read_mic() { // unsigned raw
    return ADC1_GetConversionResult();
}

/* public functions */
void app_init() {
    sei();
    
    // MCC Melody don't generate the enum...
    ADC0_StartConversion(ADC_MUXPOS_AIN8_gc); // vbatt_sense
    ADC1_StartConversion(ADC_MUXPOS_AIN3_gc); // mic
    
    reg_en_SetHigh();
    _delay_ms(100);
    
    for (uint8_t i = 0; i < 128; i++) {
        TWI0_Write(i, NULL, 0);
        while (TWI0_IsBusy());
        printf("%d %d\n", i, TWI0_ErrorGet());
    }
    
    // TODO accel/mag w/ int
    // TODO nrf24 w/ int
    // TODO write the full app
}

void app_loop() {
    ws2812b_write(2, 2, 2);
    printf("%d %d\n", app_read_vbatt(), app_read_mic());
    _delay_ms(10);
}
