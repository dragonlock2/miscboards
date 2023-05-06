#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dbg.h"
#include "btn.h"
#include "anim.h"
#include "audio.h"
#include "pwr.h"
#include "imu.h"

/* private data */
struct {
    uint16_t tick_ctr;
    volatile bool tick;

    // TODO cleanup
    bool on;
} main_data;

/* private helpers */
static void run50Hz() {
    btn_run50Hz();
    anim_step(); // can call more times to speed up

    // TODO cleanup
    if (btn_rising()) {
        main_data.on = !main_data.on;
        if (main_data.on) {
            anim_set(ANIM_TYPE_EXTEND,  0, 32, 48, 3);
        } else {
            anim_set(ANIM_TYPE_RETRACT, 0, 32, 48, 3);
        }
    }
}

static void runPWMFreq() {
    main_data.tick_ctr++;
    if (main_data.tick_ctr == (PWM_FREQ / 50)) {
        main_data.tick_ctr = 0;
        main_data.tick = true;
    }
}

/* public functions */
int main(void) {
    // divider = 1 => 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    cli();
    pwr_init();
    dbg_init();
    btn_init();
    anim_init();
    imu_init();
    audio_init(runPWMFreq);
    sei();
    printf("booted!\r\n");

    while (1) {
        while (!main_data.tick);
        run50Hz();
        main_data.tick = false;
    }
}
