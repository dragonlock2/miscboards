#include <cstdio>
#include "btn.h"
#include "motor.h"
#include "pwr.h"
#include "rgb.h"
#include "ticker.h"
#include "vbat.h"

extern "C" int main(void) {
    volatile bool signal;
    ticker_start(1000, &signal);
    printf("booted!\r\n\n");

    pwr_sleep();
    while (true) {
        // TODO app
        signal = false;
        while (!signal);
    }
}
