#ifndef SSD1306_H
#define SSD1306_H

#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include "config.h"

class ssd1306 {
public:
    ssd1306(uint height, uint width, i2c_inst_t *i2c, uint sda, uint scl, uint8_t addr);

    void set_cursor(uint x, uint y);
    void draw_pixel(uint x, uint y, bool on);
    void draw_string(const char *s);
    void draw_char(char c);
    void clear(void);
    bool display(void); // true if prior frame done, starts next one

private:
    void send_command(uint8_t cmd);

    // config
    i2c_inst_t *i2c;
    uint8_t addr;
    uint height;
    uint width;

    // graphics
    uint x;
    uint y;
    bool ping; // indicates buffer graphics get written to
    uint8_t buffer[2][1 + MAX_SSD1306_HEIGHT * MAX_SSD1306_WIDTH / 8]; // extra byte for register
};

#endif // SSD1306_H
