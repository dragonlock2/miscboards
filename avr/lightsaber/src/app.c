#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "anim.h"
#include "btn.h"
#include "dbg.h"
#include "font.h"
#include "leds.h"
#include "imu.h"
#include "pwr.h"

/* private defines */
typedef enum {
    APP_STATE_SLEEP,
    APP_STATE_FONT_CHOICE,
    APP_STATE_IDLE,
    APP_STATE_ON,
    APP_STATE_ON_EFFECT,
    APP_STATE_OFF,
} app_state_t;

/* private data */
static struct {
    volatile bool tick;

    app_state_t state;
    uint16_t sleep_ctr;
    uint8_t swing_ctr;
} app_data;

/* private helpers */
static inline void app_sleep() {
    font_sleep();
    leds_sleep();
    pwr_sleep();
    btn_sleep();

    // draws ~0.6uA in sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();

    btn_wake();
    pwr_wake();
    leds_wake();
    font_wake();
    imu_wake();
}

static inline void app_run() {
    switch (app_data.state) {
        case APP_STATE_SLEEP:
            app_sleep();
            font_play(FONT_TYPE_BOOT);
            app_data.state = APP_STATE_FONT_CHOICE;
            break;

        case APP_STATE_FONT_CHOICE:
            if (font_done()) {
                if (btn_pressed()) {
                    font_select((font_idx() + 1) % font_num());
                    font_play(FONT_TYPE_BOOT);
                    anim_set(ANIM_TYPE_SOLID, font_r(), font_g(), font_b(), 1, 1);
                } else {
                    app_data.sleep_ctr = 0;
                    app_data.state = APP_STATE_IDLE;
                    anim_set(ANIM_TYPE_SOLID, 0, 0, 0, 1, 1);
                }
            }
            break;

        case APP_STATE_IDLE:
            if (btn_short_pressed()) {
                font_play(FONT_TYPE_ON);
                anim_set(ANIM_TYPE_EXTEND, font_r(), font_g(), font_b(), font_upscale(), font_speedup());
                app_data.state = APP_STATE_ON_EFFECT;
            } else if (btn_hold()) {
                app_data.state = APP_STATE_FONT_CHOICE;
            } else {
                app_data.sleep_ctr++;
                if (app_data.sleep_ctr > 500) { // ~10s
                    app_data.state = APP_STATE_SLEEP;
                }
            }
            break;

        case APP_STATE_ON:
            if (btn_short_pressed()) {
                font_play(FONT_TYPE_OFF);
                anim_set(ANIM_TYPE_RETRACT, font_r(), font_g(), font_b(), font_upscale(), font_speedup());
                app_data.state = APP_STATE_OFF;
            } else {
                imu_data_S a;
                imu_read(&a);
                int32_t x = a.x, y = a.y, z = a.z;
                int32_t g = x*x + y*y + z*z;

                if (g > 25000000) {
                    font_play(FONT_TYPE_CLASH);
                    app_data.state = APP_STATE_ON_EFFECT;
                } else if (g > 17000000) {
                    if (app_data.swing_ctr > 4) {
                        font_play(FONT_TYPE_SWING);
                        app_data.state = APP_STATE_ON_EFFECT;
                    } else {
                        app_data.swing_ctr++;
                    }
                } else {
                    app_data.swing_ctr = 0;
                }
            }
            break;

        case APP_STATE_ON_EFFECT:
            if (anim_done() && font_done()) {
                app_data.swing_ctr = 0;
                app_data.state = APP_STATE_ON;
            }
            break;

        case APP_STATE_OFF:
            if (anim_done() && font_done()) {
                app_data.sleep_ctr = 0;
                app_data.state = APP_STATE_IDLE;
            }
            break;

        default:
            app_data.state = APP_STATE_SLEEP;
            break;
    }
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

    app_data.state = APP_STATE_SLEEP;

    while (1) {
        while (!app_data.tick);

        btn_run();
        anim_step();
        app_run();

        app_data.tick = false;
    }
}
