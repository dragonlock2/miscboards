#ifndef CONFIG_H
#define CONFIG_H

#include <tusb.h>
#include <pico/stdlib.h>

#define MAX_ROWS (8) // limit of non-consecutive WS2812B pins w/ PIO
#define MAX_COLS (16)

#define MAX_SSD1306_HEIGHT (64)
#define MAX_SSD1306_WIDTH  (128)

// board-specific config
#define DEBOUNCE_TIME (5) //ms
static const uint NUM_ROWS = 5;
static const uint NUM_COLS = 9;
static const uint ROW_PINS[NUM_ROWS] = { 0, 2, 4, 6, 8 };
static const uint COL_PINS[NUM_COLS] = { 16, 17, 18, 15, 14, 13, 12, 11, 10 };

#define LED_COL_REVERSE (true)
static const uint LED_PINS[NUM_ROWS] = { 1, 3, 5, 7, 9 };

#define OLED_HEIGHT (64)
#define OLED_WIDTH  (128)
#define OLED_I2C    (i2c0)
#define OLED_SDA    (20)
#define OLED_SCL    (21)
#define OLED_ADDR   (0x3C)
#define OLED_FREQ   (3000000) // Hz

#define SLEEP_PIN (28)

#define ENCODER_PIN_A   (26)
#define ENCODER_PIN_B   (27)
#define ENCODER_REVERSE (false)

// keyboard HID
static const uint8_t KEYMAP[MAX_ROWS][MAX_COLS] = {
    {HID_KEY_GRAVE,     HID_KEY_1,    HID_KEY_2,    HID_KEY_3,     HID_KEY_4,     HID_KEY_5,     HID_KEY_6,     HID_KEY_7,     HID_KEY_8},
    {HID_KEY_TAB,       HID_KEY_Q,    HID_KEY_W,    HID_KEY_E,     HID_KEY_R,     HID_KEY_T,     HID_KEY_Y,     HID_KEY_U,     HID_KEY_I},
    {HID_KEY_CAPS_LOCK, HID_KEY_A,    HID_KEY_S,    HID_KEY_D,     HID_KEY_F,     HID_KEY_G,     HID_KEY_H,     HID_KEY_J,     HID_KEY_K},
    {HID_KEY_NONE,      HID_KEY_Z,    HID_KEY_X,    HID_KEY_C,     HID_KEY_V,     HID_KEY_B,     HID_KEY_N,     HID_KEY_M,     HID_KEY_COMMA},
    {HID_KEY_NONE,      HID_KEY_NONE, HID_KEY_NONE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_BACKSPACE},
};

static const uint8_t KEYMODMAP[MAX_ROWS][MAX_COLS] = {
    {0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {KEYBOARD_MODIFIER_LEFTCTRL,  KEYBOARD_MODIFIER_LEFTALT, KEYBOARD_MODIFIER_LEFTGUI, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

static const uint8_t ENCMAP[2] = { HID_KEY_NONE, HID_KEY_NONE };

// consumer HID
static const uint16_t CONSUMER_KEYMAP[MAX_ROWS][MAX_COLS] = {
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
};

static const uint16_t CONSUMER_ENCMAP[2] = { HID_USAGE_CONSUMER_VOLUME_DECREMENT, HID_USAGE_CONSUMER_VOLUME_INCREMENT };

// TODO mouse HID support

#endif // CONFIG_H