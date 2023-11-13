#include <cstdbool>
#include <cstdio>
#include <unistd.h>
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

    // TODO proper rpc
    // very rudimentary rpc for testing
    // while (true) {
    //     uint8_t b[256];

    //     read(stdin->_file, b, 2);
    //     if (b[0] != 0x69) { continue; }
    //     switch (b[1]) {
    //         case 0x01: { // erase
    //             read(stdin->_file, b, 8);
    //             uint32_t addr = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    //             uint32_t len  = (b[4] << 24) | (b[5] << 16) | (b[6] << 8) | b[7];
    //             fpga_erase(addr, len);
    //             uint8_t t[5] = {0x69, 0x00, 0x00, 0x00, 0x00};
    //             write(stdout->_file, t, 5);
    //             break;
    //         }

    //         case 0x02: { // write
    //             read(stdin->_file, b, 4);
    //             uint32_t addr = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    //             read(stdin->_file, b, 256);
    //             fpga_write(addr, b);
    //             uint8_t t[5] = {0x69, 0x00, 0x00, 0x00, 0x00};
    //             write(stdout->_file, t, 5);
    //             break;
    //         }

    //         case 0x03: { // read
    //             read(stdin->_file, b, 4);
    //             uint32_t addr = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    //             fpga_read(addr, b);
    //             uint8_t t[5] = {0x69, 0x00, 0x00, 0x01, 0x00}; // 256-byte payload
    //             write(stdout->_file, t, 5);
    //             write(stdout->_file, b, 256);
    //             break;
    //         }
    //     }
    // }

    fpga_on();

    bool i = false;
    while (true) {
        rgb_write(i, 0, fpga_booted());
        i = !i;

        while (!flag);
        flag = false;
    }

    // TODO use f4pga, make blinky
    // TODO test 2 basic spis, do a byte delay

    // TODO rewrite in cpp fashion, templates!
    // TODO add freertos
}
