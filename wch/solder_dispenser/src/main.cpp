#include <cstdio>
#include "btn.h"
#include "motor.h"
#include "rgb.h"
#include "ticker.h"
#include "vbat.h"

extern "C" int main(void) {
    // TODO ripple, sleep (exti wake)
    volatile bool signal;
    ticker_start(1000, &signal);
    printf("booted!\r\n");

    while (true) {
        // TODO app
        signal = false;
        while (!signal);
    }
}
