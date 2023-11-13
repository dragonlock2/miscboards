#include <cstdbool>
#include <cstdio>
#include "btn.h"
#include "fpga.h"
#include "rgb.h"
#include "tick.h"

static volatile bool flag = false;
static void tick(void) {
    flag = true;
}

extern "C" int main(void) {
    tick_init(2, tick);
    printf("booted!\r\n");

    bool i = false;
    while (true) {
        rgb_write(i, 0, 0);
        i = !i;

        while (!flag);
        flag = false;
    }

    // TODO test fpga flashing
    // TODO test fpga basic boot
    // TODO tests 2 basic spis
    // TODO rewrite in cpp fashion, templates!
    // TODO add freertos
}
