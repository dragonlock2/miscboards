#include "mcc_generated_files/mcc.h"
#include "ssd1306.h"

/* private defines */
#define SSD1306_ADDR    0x3C
#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT    64
#define SSD1306_FONT_W     4
#define SSD1306_FONT_H     6

// font converted from https://blog.mclemon.io/hacking-a-tiny-new-font-for-the-ssd1306-128x64-oled-screen
static const uint32_t font[128] = { // 4x6 font (including spaces) stored in lowest 24 bits, column major order
    0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
    0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
    0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,
    0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f45f,0x0001f7df,
    0x00000000,0x000005c0,0x00003003,0x0001f29f,0x000057ca,0x00012109,0x0001c5cf,0x000000c0,
    0x00011380,0x00000391,0x00005085,0x00004384,0x00000210,0x00004104,0x00000400,0x00003118,
    0x0000f45e,0x000007c2,0x00012559,0x0000a551,0x0001f107,0x00009557,0x0001d55e,0x00003159,
    0x0001f55f,0x0000f557,0x00000280,0x00000290,0x00011284,0x0000a28a,0x00004291,0x00003541,
    0x0001654e,0x0001e15e,0x0000a55f,0x0001144e,0x0000e45f,0x0001555f,0x0000515f,0x0001d54e,
    0x0001f11f,0x000117d1,0x0000f408,0x0001b11f,0x0001041f,0x0001f19f,0x0001f39f,0x0000e44e,
    0x0000215f,0x0001e64e,0x0001635f,0x00009552,0x000017c1,0x0001f40f,0x00007607,0x0001f31f,
    0x0001b11b,0x00003703,0x00013559,0x0001145f,0x00008102,0x0001f451,0x00002042,0x00010410,
    0x00000081,0x0001c59a,0x0000c49f,0x0001248c,0x0001f48c,0x0001668c,0x00005784,0x0001ea8c,
    0x0001c09f,0x00000740,0x0001d810,0x0001231f,0x000107d1,0x0001e39e,0x0001c09e,0x0000c48c,
    0x0000c4be,0x0003e48c,0x0000209c,0x0000a794,0x000127c2,0x0001e40e,0x0000e60e,0x0001e71e,
    0x00012312,0x0001ea06,0x0001679a,0x000116c4,0x000007c0,0x000046d1,0x00002184,0x0001f7df,
};

typedef struct {
    uint8_t reg;
    uint8_t buff[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    uint16_t x, y;
} ssd1306_data_S;
static ssd1306_data_S ssd1306_data;

/* private helpers */
void ssd1306_write_command(uint8_t comm) {
    uint8_t buf[2] = {0x00, comm};
    while (!OLED_Open(SSD1306_ADDR));
    OLED_SetBuffer(buf, sizeof buf);
    OLED_MasterOperation(false);
    while (TWI1_BUSY == OLED_Close());
}

/* public functions */
void ssd1306_init() {
    TWBR1 = 2; // MCC doesn't do this... (800kHz)

    ssd1306_write_command(0xAE); // display off
    ssd1306_write_command(0x00); // set lower column start address
    ssd1306_write_command(0x10); // set higher column start address
    ssd1306_write_command(0x20); // set memory addressing mode
    ssd1306_write_command(0x00); // - horizontal addressing mode
    ssd1306_write_command(0x40); // set display start line
    ssd1306_write_command(0x81); // set contrast control
    ssd1306_write_command(0xFF); // - max contrast
    ssd1306_write_command(0xA0); // set segment re-map (flipped)
    ssd1306_write_command(0xA4); // entire display on (follows RAM)
    ssd1306_write_command(0xA6); // set normal display (non-inverted)
    ssd1306_write_command(0xA8); // set multiplex ratio
    ssd1306_write_command(0x3F); // - 63
    ssd1306_write_command(0xB0); // set page start address
    ssd1306_write_command(0xC0); // set COM output scan direction (flipped)
    ssd1306_write_command(0xD3); // set display offset
    ssd1306_write_command(0x00); // - no offset
    ssd1306_write_command(0xD5); // set display clock divide ratio/oscillator frequency
    ssd1306_write_command(0xF0); // - divide ratio = 1, oscillator freq = 16 (max)
    ssd1306_write_command(0xD9); // set pre-charge period
    ssd1306_write_command(0x22); // - 34
    ssd1306_write_command(0xDA); // set COM pins hardware configuration
    ssd1306_write_command(0x12); // - idk actually
    ssd1306_write_command(0xDB); // set Vcomh
    ssd1306_write_command(0x20); // - 0.77xVcc
    ssd1306_write_command(0x8D); // charge pump setting
    ssd1306_write_command(0x14); // - enable
    ssd1306_write_command(0xAF); // display on

    ssd1306_data.x = 0;
    ssd1306_data.y = 0;
}

void ssd1306_set_cursor(uint16_t x, uint16_t y) {
    ssd1306_data.x = x;
    ssd1306_data.y = y;
}

void ssd1306_draw_string(const char *s) {
    while (*s) {
        ssd1306_draw_char(*s);
        ssd1306_data.x += SSD1306_FONT_W;
        if (ssd1306_data.x > SSD1306_WIDTH - SSD1306_FONT_W) {
            ssd1306_data.x = 0;
            ssd1306_data.y += SSD1306_FONT_H;
        }
        s++;
    }
}

void ssd1306_draw_char(char c) {
    // definitely slower than using byte high fonts, but can squeeze more in
    if (c > 127 || c < 0) { // default should be unsigned char
        c = 0; // just draw this if non-supported char
    }

    uint32_t b = font[(uint32_t) c];
    for (uint8_t x = 0; x < SSD1306_FONT_W; x++) {
        for (uint8_t y = 0; y < SSD1306_FONT_H; y++) {
            ssd1306_draw_pixel(ssd1306_data.x + x, ssd1306_data.y + y, b & 0x1);
            b >>= 1;
        }
    }
}

void ssd1306_draw_pixel(uint16_t x, uint16_t y, bool on) {
    if (on) {
        ssd1306_data.buff[x + y / 8 * SSD1306_WIDTH] |= 1 << (y % 8);
        } else {
        ssd1306_data.buff[x + y / 8 * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void ssd1306_clear_display() {
    memset(ssd1306_data.buff, 0x00, sizeof(ssd1306_data.buff));
}

void ssd1306_display() {
    ssd1306_data.reg = 0x40;
    while (!OLED_Open(SSD1306_ADDR));
    OLED_SetBuffer(&ssd1306_data.reg, 1 + SSD1306_HEIGHT * SSD1306_WIDTH / 8);
    OLED_MasterOperation(false);
    while (TWI1_BUSY == OLED_Close());
}
