#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dbg.h"
#include "leds.h"
#include "btn.h"
#include "anim.h"

/* private defines */
#define PWM_FREQ  (25000) // Hz
#define LOOP_FREQ (50) // Hz

/* private data */
typedef struct {
    uint16_t tick_ctr;
    volatile bool tick;

    // TODO cleanup
    bool on;
} main_data_S;
static main_data_S main_data;

/* private helpers */
static void run50Hz() {
    btn_run50Hz();
    anim_step(); // can call more times to speed up

    // TODO cleanup
    if (btn_rising()) {
        main_data.on = !main_data.on;
        if (main_data.on) {
            anim_set(true,  ANIM_TYPE_EXTEND,  0, 0, 255, 2);
            anim_set(false, ANIM_TYPE_EXTEND,  0, 0, 255, 2);
        } else {
            anim_set(true,  ANIM_TYPE_RETRACT, 0, 0, 255, 2);
            anim_set(false, ANIM_TYPE_RETRACT, 0, 0, 255, 2);
        }
    }
}

static void clk_init() {
    // divider = 1 => 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    // setup TCA0
    TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV2_gc | 0x01; // ENABLE=1
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.INTCTRL = 0x01; // OVF=1
    TCA0.SINGLE.PER     = F_CPU / 2 / PWM_FREQ - 1;

    main_data.tick_ctr = 0;
    main_data.tick = false;
}

ISR(TCA0_OVF_vect) {
    // 25kHz ISR
    TCA0.SINGLE.INTFLAGS = 0x01; // clear OVF

    main_data.tick_ctr++;
    if (main_data.tick_ctr == (PWM_FREQ / LOOP_FREQ)) {
        main_data.tick_ctr = 0;
        main_data.tick = true;
    }
}

/* public functions */
int main(void) {
    // HW init
    cli();
    clk_init();
    dbg_init();
    btn_init();
    anim_init();
    sei();
    printf("booted!\r\n");

    // loop
    while (1) {
        // TODO measure performance
        while (!main_data.tick);
        run50Hz();
        main_data.tick = false;
    }
}
