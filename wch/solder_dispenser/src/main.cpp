#include <cstdio>
#include "btn.h"
#include "extrude.h"
#include "motor.h"
#include "pwr.h"
#include "rgb.h"
#include "ticker.h"
#include "vbat.h"

// 0402 ~= 0.05mm^3
// 0603 ~= 0.11mm^3
#define DEFAULT_STEP  (2)
#define MAX_STEP      (8)
#define MM3_PER_STEP  (0.025f)
#define LOW_BATT      (3400) // mV
#define SLEEP_TIME    (10000) // ms

static struct {
    uint32_t amt;
    uint32_t sleep_ctr;
    uint8_t  batt_ctr;
} data;

extern "C" int main(void) {
    volatile bool signal;
    ticker_start(1000, &signal);
    printf("booted!\r\n\n");

    data.amt       = DEFAULT_STEP;
    data.sleep_ctr = SLEEP_TIME;

    while (true) {
        btn_run();
        extrude_run();

        bool pwr_good = vbat_read() > LOW_BATT;

        // process user commands
        if (pwr_good && btn_pressed(btn::dispense)) {
            extrude_dispense(data.amt * MM3_PER_STEP);
        } else if (btn_pressed(btn::up)) {
            if (data.amt < MAX_STEP) {
                data.amt += 1;
            }
        } else if (btn_pressed(btn::down)) {
            if (data.amt > 0) {
                data.amt -= 1;
            }
        }

        // setting indication
        if (!extrude_idle()) {
            rgb_write(0, 0, 32);
            data.sleep_ctr = SLEEP_TIME;
        } else if (pwr_good) {
            uint8_t r = 32 * (MAX_STEP - data.amt) / MAX_STEP;
            uint8_t g = 16  * data.amt / MAX_STEP;
            rgb_write(r, g, 0);
        } else {
            rgb_write((data.batt_ctr++) / 8, 0, 0);
        }

        data.sleep_ctr--;
        if (data.sleep_ctr == 0) {
            pwr_sleep();
            data.sleep_ctr = SLEEP_TIME;
        }

        signal = false;
        while (!signal);
    }
}
