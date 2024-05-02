#include <cstdio>
#include "btn.h"
#include "extrude.h"
#include "motor.h"
#include "pwr.h"
#include "rgb.h"
#include "ticker.h"
#include "vbat.h"

#define DEFAULT_STEP  (2)
#define MAX_STEP      (8)
#define MM3_PER_STEP  (0.1f)
#define LOW_BATT_HI   (3600) // mV
#define LOW_BATT_LO   (3400) // mV
#define SLEEP_TIME    (10000) // ms

static struct {
    uint32_t amt;
    uint32_t sleep_ctr;
    uint8_t  batt_ctr;
    bool pwr_good;
} data;

extern "C" int main(void) {
    volatile bool signal;
    ticker_start(1000, &signal);
    printf("booted!\r\n\n");

    data.amt       = DEFAULT_STEP;
    data.sleep_ctr = SLEEP_TIME;
    data.pwr_good  = true;

    while (true) {
        btn_run();
        extrude_run();

        if (data.pwr_good) {
            data.pwr_good = vbat_read() > LOW_BATT_LO;
        } else {
            data.pwr_good = vbat_read() > LOW_BATT_HI;
        }

        // process user commands
        if (data.pwr_good && btn_read(btn::dispense)) {
            extrude_dispense();
        } else if (btn_pressed(btn::up)) {
            if (data.amt < MAX_STEP) {
                data.amt += 1;
            }
        } else if (btn_pressed(btn::down)) {
            if (data.amt > 0) {
                data.amt -= 1;
            }
        }
        extrude_set(data.amt * MM3_PER_STEP);

        // setting indication
        if (!extrude_idle()) {
            rgb_write(0, 0, 32);
            data.sleep_ctr = SLEEP_TIME;
        } else if (data.pwr_good) {
            uint8_t r = 32 * (MAX_STEP - data.amt) / MAX_STEP;
            uint8_t g = 16  * data.amt / MAX_STEP;
            rgb_write(r, g, 0);
        } else {
            rgb_write((data.batt_ctr++) / 8, 0, 0);
        }

        // sleep counter
        data.sleep_ctr--;
        if (data.sleep_ctr == 0) {
            rgb_write(0, 0, 0);
            pwr_sleep();
            data.sleep_ctr = SLEEP_TIME;
        }

        signal = false;
        while (!signal);
    }
}
