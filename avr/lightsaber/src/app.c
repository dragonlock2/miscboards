#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "anim.h"
#include "btn.h"
#include "dbg.h"
#include "font.h"
#include "imu.h"
#include "pwr.h"

/* private data */
static struct {
    volatile bool tick;
} app_data;

/* private helpers */
static void app_sleep() {
    font_sleep();
    btn_sleep();
    pwr_sleep();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();

    pwr_wake();
    btn_wake();
    font_wake();
    imu_wake();
}

static void app_ticker() {
    app_data.tick = true;
}

/* public functions */
int main() {
    // divider = 1 => 20MHz
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = 0x00;

    cli();
    pwr_init();
    dbg_init();
    btn_init();
    anim_init();
    imu_init();
    font_init(app_ticker, 50); // Hz
    sei();
    printf("booted!\r\n");

    // TODO try sleeping and measure current, then wake and make sure everything works
    app_sleep();
    printf("hello!\r\n");

    while (1) {
        while (!app_data.tick);

        btn_run();
        anim_step();

        app_data.tick = false;
    }
}
