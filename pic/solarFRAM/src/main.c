#include <xc.h>
#include "config.h"
#include "i2c.h"
#include "led.h"
#include "flash.h"

#define FLASH_ADDR (0x69)

int main(void) {
    OSCCONbits.IRCF = 0b1010; // select FOSC = 500kHz
    
    i2c_init();

    uint64_t ram_ctr = 0, flash_ctr = 0;
    while (1) {
        bool res = true;
        led_off();

        // read, increment, and write back counter
        res &= flash_read(FLASH_ADDR, (uint8_t*) &ram_ctr, sizeof ram_ctr);
        ram_ctr++;
        res &= flash_write(FLASH_ADDR, (uint8_t*) &ram_ctr, sizeof ram_ctr);

        __delay_ms(100);
        
        // read back counter and check
        res &= flash_read(FLASH_ADDR, (uint8_t*) &flash_ctr, sizeof flash_ctr);
        res &= ram_ctr == flash_ctr;

        // every once in awhile do something fun
        if (ram_ctr % 69 == 0) {
            for (int i = 0; i < 6; i++) {
                led_write(true);
                __delay_ms(50);
                led_write(false);
                __delay_ms(50);
            }
        }

        led_write(res);
        __delay_ms(100);
    }
}
