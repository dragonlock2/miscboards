#include <cstring>
#include "ssd1306.h"

// font converted from https://blog.mclemon.io/hacking-a-tiny-new-font-for-the-ssd1306-128x64-oled-screen
#define FONT_W (4)
#define FONT_H (6)
static const uint32_t FONT[128] = { // 4x6 font (including spaces) stored in lowest 24 bits, column major order
    0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f,
    0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f,
    0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f,
    0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f45f, 0x0001f7df,
    0x00000000, 0x000005c0, 0x00003003, 0x0001f29f, 0x000057ca, 0x00012109, 0x0001c5cf, 0x000000c0,
    0x00011380, 0x00000391, 0x00005085, 0x00004384, 0x00000210, 0x00004104, 0x00000400, 0x00003118,
    0x0000f45e, 0x000007c2, 0x00012559, 0x0000a551, 0x0001f107, 0x00009557, 0x0001d55e, 0x00003159,
    0x0001f55f, 0x0000f557, 0x00000280, 0x00000290, 0x00011284, 0x0000a28a, 0x00004291, 0x00003541,
    0x0001654e, 0x0001e15e, 0x0000a55f, 0x0001144e, 0x0000e45f, 0x0001555f, 0x0000515f, 0x0001d54e,
    0x0001f11f, 0x000117d1, 0x0000f408, 0x0001b11f, 0x0001041f, 0x0001f19f, 0x0001f39f, 0x0000e44e,
    0x0000215f, 0x0001e64e, 0x0001635f, 0x00009552, 0x000017c1, 0x0001f40f, 0x00007607, 0x0001f31f,
    0x0001b11b, 0x00003703, 0x00013559, 0x0001145f, 0x00008102, 0x0001f451, 0x00002042, 0x00010410,
    0x00000081, 0x0001c59a, 0x0000c49f, 0x0001248c, 0x0001f48c, 0x0001668c, 0x00005784, 0x0001ea8c,
    0x0001c09f, 0x00000740, 0x0001d810, 0x0001231f, 0x000107d1, 0x0001e39e, 0x0001c09e, 0x0000c48c,
    0x0000c4be, 0x0003e48c, 0x0000209c, 0x0000a794, 0x000127c2, 0x0001e40e, 0x0000e60e, 0x0001e71e,
    0x00012312, 0x0001ea06, 0x0001679a, 0x000116c4, 0x000007c0, 0x000046d1, 0x00002184, 0x0001f7df,
};

ssd1306::ssd1306(uint height, uint width, i2c_inst_t *i2c, uint sda, uint scl, uint8_t addr)
:
     i2c(i2c), addr(addr), height(height), width(width), x(0), y(0), ping(false)
{
    i2c_init(i2c0, 1000000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);

    send_command(0xAE); // display off
    send_command(0x00); // set lower column start address
    send_command(0x10); // set higher column start address
    send_command(0x20); // set memory addressing mode
    send_command(0x00); // - horizontal addressing mode
    send_command(0x40); // set display start line
    send_command(0x81); // set contrast control
    send_command(0xFF); // - max contrast
    send_command(0xA0); // set segment re-map (flipped)
    send_command(0xA4); // entire display on (follows RAM)
    send_command(0xA6); // set normal display (non-inverted)
    send_command(0xA8); // set multiplex ratio
    send_command(0x3F); // - 63
    send_command(0xB0); // set page start address
    send_command(0xC0); // set COM output scan direction (flipped)
    send_command(0xD3); // set display offset
    send_command(0x00); // - no offset
    send_command(0xD5); // set display clock divide ratio/oscillator frequency
    send_command(0xF0); // - divide ratio = 1, oscillator freq = 16 (max)
    send_command(0xD9); // set pre-charge period
    send_command(0x22); // - 34
    send_command(0xDA); // set COM pins hardware configuration
    send_command(0x12); // - idk actually
    send_command(0xDB); // set Vcomh
    send_command(0x20); // - 0.77xVcc
    send_command(0x8D); // charge pump setting
    send_command(0x14); // - enable
    send_command(0xAF); // display on

    memset(buffer, 0, sizeof buffer);
}

void ssd1306::set_cursor(uint x, uint y) {
    this->x = x;
    this->y = y;
}

void ssd1306::draw_pixel(uint x, uint y, bool on) {
    if (on) {
        buffer[ping][1 + x + y / 8 * width] |= 1 << (y % 8);
    } else {
        buffer[ping][1 + x + y / 8 * width] &= ~(1 << (y % 8));
    }
}

void ssd1306::draw_string(const char *s) {
    while (*s) {
        draw_char(*s);
        x += FONT_W;
        if (x + FONT_W > width) {
            x = 0;
            y += FONT_H;
            if (y + FONT_H > height) {
                y = 0; // wrap
            }
        }
        s++;
    }
}

void ssd1306::draw_char(char c) {
    if (c > 127 || c < 0) { c = 0; }
    uint32_t b = FONT[(uint) c];
    for (uint i = 0; i < FONT_W; i++) {
        for (uint j = 0; j < FONT_H; j++) {
            draw_pixel(x + i, y + j, b & 0x1);
            b >>= 1;
        }
    }
}

void ssd1306::clear(void) {
    memset(buffer[ping], 0, sizeof buffer[0]);
}

bool ssd1306::display(void) {
    bool ret = false;
    if (true) { // TODO DMA transfer, check semaphore
        buffer[ping][0] = 0x40;
        i2c_write_blocking(i2c, addr, buffer[ping], 1 + height * width / 8, false);

        // copy buffer over
        memcpy(buffer[!ping], buffer[ping], sizeof buffer[0]);
        ping = !ping;
        ret = true;
    }
    return ret;
}

void ssd1306::send_command(uint8_t cmd) {
    uint8_t buf[2] = { 0x00, cmd };
    i2c_write_blocking(i2c, addr, buf, 2, false);
}
