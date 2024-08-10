#include <cstdio>
#include "mdio.h"
#include "rgb.h"

extern "C" int main(void) {
    uint16_t tmp;

    // reset phys
    mdio_write(PHY_ADIN1100, 0x00, 0x8000);
    while (mdio_read(PHY_ADIN1100, 0x00) & 0x8000);
    while (!(mdio_read_c45(PHY_ADIN1100, 0x1e, 0x8818) & 0x0001));

    mdio_write(PHY_RTL8201F, 0x00, 0x8000);
    while (mdio_read(PHY_RTL8201F, 0x00) & 0x8000);

    // rtl8201f led setting
    mdio_write(PHY_RTL8201F, 0x1f, 0x0007);
    tmp = mdio_read(PHY_RTL8201F, 0x11);
    mdio_write(PHY_RTL8201F, 0x11, tmp | 0x0018); // led0: act, led1: link
    tmp = mdio_read(PHY_RTL8201F, 0x13);
    mdio_write(PHY_RTL8201F, 0x13, tmp | 0x0008); // custom led
    mdio_write(PHY_RTL8201F, 0x1f, 0x0000);

    // rtl8201f use 10base-t
    tmp = mdio_read(PHY_RTL8201F, 0x04);
    mdio_write(PHY_RTL8201F, 0x04, tmp & ~(0x01a0));
    tmp = mdio_read(PHY_RTL8201F, 0x00);
    mdio_write(PHY_RTL8201F, 0x00, tmp | 0x0200);

    // adin1100 led setting
    tmp = mdio_read_c45(PHY_ADIN1100, 0x1e, 0x8c56);
    mdio_write_c45(PHY_ADIN1100, 0x1e, 0x8c56, tmp & ~0x000e); // enable led1
    tmp = mdio_read_c45(PHY_ADIN1100, 0x1e, 0x8c83);
    mdio_write_c45(PHY_ADIN1100, 0x1e, 0x8c83, tmp | 0x0005); // active-high
    tmp = mdio_read_c45(PHY_ADIN1100, 0x1e, 0x8c82);
    mdio_write_c45(PHY_ADIN1100, 0x1e, 0x8c82, (tmp & ~(0x1f1f)) | 0x0304); // led0: act, led1: link

    // adin1100 power on
    mdio_write_c45(PHY_ADIN1100, 0x1e, 0x8812, 0x0000);
    while (mdio_read_c45(PHY_ADIN1100, 0x1e, 0x8818) & 0x0002);

    printf("booted!\r\n");
    while (true) {
        tmp = mdio_read_c45(PHY_ADIN1100, 0x07, 0x0201);
        if (tmp & 0x0004) {
            int ms = (mdio_read_c45(PHY_ADIN1100, 0x07, 0x8001) >> 5) & 0x3;
            if (ms == 2) {
                rgb_write(0, 1, 0); // follower
            } else { // == 3
                rgb_write(0, 0, 1); // leader
            }
        } else {
            rgb_write(1, 0, 0);
        }
    }
}
