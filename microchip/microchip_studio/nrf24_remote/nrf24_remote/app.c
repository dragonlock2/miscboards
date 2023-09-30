#define F_CPU 20000000L

#include <avr/sleep.h>
#include <util/delay.h>
#include "mcc_generated_files/system/system.h"
#include "ws2812b.h"
#include "lsm303ah.h"
#include "rf24.h"
#include "app.h"

/* private defines */
#define RF24_ADDR "69420"
#define RF24_CHANNEL 69

#define VBATT_CUTOFF  (3500) // mV
#define SLEEP_TIMEOUT (5000) // ms
#define KEEPON_TIME   (3000) // ms

typedef struct {
    uint8_t btn0 : 1;
    uint8_t btn1 : 1;
    uint8_t btn2 : 1;
    uint8_t rsvd : 4;
    uint8_t mag  : 1; // 0=accel, 1=mag
    int16_t x; // little endian!
    int16_t y;
    int16_t z;
    uint8_t audio[22]; // 22kHz audio
} __attribute__((packed)) app_packet_S;

typedef struct {
    bool keepon;
    uint16_t keepon_ctr;
    uint16_t sleep_ctr;
    
    bool tx_ok;
    bool send_mag;
    uint8_t audio_ctr;    
    bool sending_pkt0;
    app_packet_S pkt[2];
} app_data_S;
static app_data_S app_data;

static volatile bool ticker;

/* private helpers */
static uint16_t app_read_vbatt() { // mV
    return ADC0_GetConversionResult() * 64 / 13; // * 2 * 2500mV / 1023 (close enough)
}

static uint8_t app_read_mic() { // unsigned raw
    return ADC1_GetConversionResult();
}

static void app_wakeup() {
    vbatt_sense_ResetPullUp();
    btn0_DisableInterruptOnChange();
    btn1_DisableInterruptOnChange();
    btn2_DisableInterruptOnChange();
    reg_en_SetHigh();
    _delay_ms(250);
    lsm303ah_init();
    rf24_init((uint8_t*) RF24_ADDR, RF24_CHANNEL);
    rf24_start_tx((uint8_t*) RF24_ADDR);
    ws2812b_write(69, 0, 0);
    app_data.tx_ok = false;
}

static void app_sleep() {
    vbatt_sense_SetPullUp(); // pin floating away from rails causes excess current draw
    btn0_EnableInterruptForLowLevelSensing();
    btn1_EnableInterruptForLowLevelSensing();
    btn2_EnableInterruptForLowLevelSensing();
    CE_SetLow();
    reg_en_SetLow();
    app_data.sleep_ctr = 0;
    sleep_mode();
    app_wakeup();
}

static void app_ticker() {
    app_packet_S *pkt = &app_data.pkt[app_data.sending_pkt0 ? 1 : 0];
    pkt->audio[app_data.audio_ctr++] = app_read_mic();
    if (app_data.audio_ctr == sizeof(app_data.pkt[0].audio)) {
        app_data.audio_ctr = 0;
        app_data.sending_pkt0 = !app_data.sending_pkt0;
        ticker = true;
    }
}

/* public functions */
void app_init() {
    // MCC Melody don't generate the enum...
    ADC0_StartConversion(ADC_MUXPOS_AIN8_gc); // vbatt_sense
    ADC1_StartConversion(ADC_MUXPOS_AIN3_gc); // mic

    app_data.pkt[0].rsvd = 0;
    app_data.pkt[1].rsvd = 0;
    TCA0_OverflowCallbackRegister(app_ticker);
    app_wakeup();
}

void app_loop() {
    while (!ticker);
    
    // check battery voltage
    while (app_read_vbatt() < 3500) {
        ws2812b_write(42, 0, 0);
        _delay_ms(1000);
        ws2812b_write(0, 0, 0);
        _delay_ms(1000);
    }
    
    // update status led
    if (rf24_irq_valid()) {
        bool tx_ok = rf24_tx_status();
        if (tx_ok != app_data.tx_ok) {
            if (tx_ok) {
                ws2812b_write(0, 69, app_data.keepon ? 69 : 0);
            } else {
                ws2812b_write(69, 0, app_data.keepon ? 69 : 0);
            }
        }
        app_data.tx_ok = tx_ok;
    }
    
    // assemble packet
    app_packet_S *pkt = &app_data.pkt[app_data.sending_pkt0 ? 0 : 1];
    pkt->btn0 = btn0_GetValue() ? 0 : 1;
    pkt->btn1 = btn1_GetValue() ? 0 : 1;
    pkt->btn2 = btn2_GetValue() ? 0 : 1;
    
    lsm303ah_result_S accel_mag;
    lsm303ah_read(&accel_mag);
    pkt->mag = app_data.send_mag;
    if (app_data.send_mag) {
        pkt->x = accel_mag.mx;
        pkt->y = accel_mag.my;
        pkt->z = accel_mag.mz;
    } else {
        pkt->x = accel_mag.ax;
        pkt->y = accel_mag.ay;
        pkt->z = accel_mag.az;
    }
    
    // send packet
    app_data.send_mag = !app_data.send_mag;
    rf24_send((uint8_t*) pkt, sizeof(app_packet_S));
    
    // check sleep counter
    app_data.sleep_ctr++;
    if (pkt->btn0 || pkt->btn1 || pkt->btn2 || INT1_GetValue()) {
        app_data.sleep_ctr = 0;
    }
    if (pkt->btn0) {
        app_data.keepon_ctr++;
        if (app_data.keepon_ctr == KEEPON_TIME) {
            app_data.keepon_ctr = 0;
            app_data.keepon = !app_data.keepon;
            if (app_data.tx_ok) {
                ws2812b_write(0, 69, app_data.keepon ? 69 : 0);
                } else {
                ws2812b_write(69, 0, app_data.keepon ? 69 : 0);
            }
        }
    } else {
        app_data.keepon_ctr = 0;
    }
    if (!app_data.keepon && app_data.sleep_ctr == SLEEP_TIMEOUT) {
        app_sleep();
    }
    
    ticker = false;
}
