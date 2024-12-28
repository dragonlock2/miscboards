#ifndef CONFIG_H
#define CONFIG_H

#include <tusb.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#define MAX_ROWS (8) // limit of non-consecutive WS2812B pins w/ PIO
#define MAX_COLS (16)

#define MAX_SSD1306_HEIGHT (64)
#define MAX_SSD1306_WIDTH  (128)

#define DEBOUNCE_TIME (5) // ms

#define NUM_LAYERS       (2)
#define UNUSED_LAYER_KEY { .row = 0xFF, .col = 0xFF }

struct key_pos {
    uint row;
    uint col;
};

struct kb_config {
    uint num_rows;
    uint num_cols;
    uint row_pins[MAX_ROWS];
    uint col_pins[MAX_COLS];

    struct {
        uint a;
        uint b;
        bool rev;
    } enc;

    struct {
        bool col_reverse;
        uint pins[MAX_ROWS];
    } led;

    struct {
        uint        height;
        uint        width;
        i2c_inst_t* i2c;
        uint        sda;
        uint        scl;
        uint8_t     addr;
        uint        freq;
    } oled;
    
    uint sleep;
};

struct kb_map {
    uint    default_layer;
    key_pos layer_keys[NUM_LAYERS];

    struct {
        uint8_t map[NUM_LAYERS][MAX_ROWS][MAX_COLS];
        uint8_t modmap[NUM_LAYERS][MAX_ROWS][MAX_COLS];
        uint8_t encmap[NUM_LAYERS][2];
    } keyboard;

    struct {
        uint8_t map[NUM_LAYERS][MAX_ROWS][MAX_COLS];
        uint8_t encmap[NUM_LAYERS][2];
    } consumer;

    // TODO mouse HID support
};

// include board-specific config
#ifndef BOARD
#warning "board not defined, defaulting to 0"
#define BOARD (0)
#endif

#if BOARD == 0
#include "configs/macropad_left.h"
#elif BOARD == 1
#include "configs/macropad_right.h"
#else
#error "select a board!"
#endif

#endif // CONFIG_H
