#include <stdbool.h>
#include <project.h>
#include "rgb.h"
#include "mdio_bb.h"
#include "dbg.h"

#define KSZ_ADDR  (0b00000)
#define DP83_ADDR (0b00001)

uint16_t dp83_read_ext(uint16_t reg) {
    mdio_write_reg(DP83_ADDR, 0x0D, 0x001F);
    mdio_write_reg(DP83_ADDR, 0x0E, reg);
    mdio_write_reg(DP83_ADDR, 0x0D, 0x401F);
    return mdio_read_reg(DP83_ADDR, 0x0E);
}

void dp83_write_ext(uint16_t reg, uint16_t data) {
    mdio_write_reg(DP83_ADDR, 0x0D, 0x001F);
    mdio_write_reg(DP83_ADDR, 0x0E, reg);
    mdio_write_reg(DP83_ADDR, 0x0D, 0x401F);
    mdio_write_reg(DP83_ADDR, 0x0E, data);
}

uint16_t dp83_read_mmd1(uint16_t reg) {
    mdio_write_reg(DP83_ADDR, 0x0D, 0x0001);
    mdio_write_reg(DP83_ADDR, 0x0E, reg);
    mdio_write_reg(DP83_ADDR, 0x0D, 0x4001);
    return mdio_read_reg(DP83_ADDR, 0x0E);
}

void dp83_write_mmd1(uint16_t reg, uint16_t data) {
    mdio_write_reg(DP83_ADDR, 0x0D, 0x0001);
    mdio_write_reg(DP83_ADDR, 0x0E, reg);
    mdio_write_reg(DP83_ADDR, 0x0D, 0x4001);
    mdio_write_reg(DP83_ADDR, 0x0E, data);
}

int main(void) {
    CyGlobalIntEnable;
    dbg_init();
    rgb_init();
    PHY_RST_Write(1);
    dbg_printf("booted!\r\n");
    
    // reset PHYs
    mdio_write_reg(KSZ_ADDR,  0x00, 0x8000);
    mdio_write_reg(DP83_ADDR, 0x00, 0x8000);
    while ((mdio_read_reg(KSZ_ADDR,  0x00) & 0x8000) ||
           (mdio_read_reg(DP83_ADDR, 0x00) & 0x8000)) {
        CyDelay(10);
    }

    // enable RMII back-to-back mode
    mdio_write_reg(KSZ_ADDR,  0x16, 0x0042);
    dp83_write_ext(0x47D, 0x0000); // undocumented register!
    
    rgb_color_E c = 0;
    while (true) {
        // alternate randomly between master and slave if not connected
        bool connected = mdio_read_reg(DP83_ADDR, 0x01) & 0x0004;
        if (!connected) {
            dp83_write_mmd1(0x0834, CY_SYS_WDT_COUNTER_REG % 2 == 0 ? 0xC000 : 0x8000); // WDT as RNG!
        }
        
        rgb(c = c == RGB_COUNT ? 0 : c + 1);
        CyDelay(25);
    }
}
