static const kb_config board_cfg = {
    .num_rows = 5,
    .num_cols = 9,
    .row_pins = { 0, 2, 4, 6, 8 },
    .col_pins = { 16, 17, 18, 15, 14, 13, 12, 11, 10 },
    .enc = {
        .a   = 26,
        .b   = 27,
        .rev = false,
    },
    .led = {
        .col_reverse = true,
        .pins = { 1, 3, 5, 7, 9 },
    },
    .oled = {
        .height = 64,
        .width  = 128,
        .i2c    = i2c0,
        .sda    = 20,
        .scl    = 21,
        .addr   = 0x3C,
        .freq   = 3000000, // Hz
    },
    .sleep = 28,
};

static const kb_map board_map = {
    .default_layer = 0,
    .layer_keys = {
        UNUSED_LAYER_KEY,
        { .row = 4, .col = 8 },
    },
    .keyboard = {
        .map = {
            { // layer 0
                { HID_KEY_ESCAPE,    HID_KEY_1,    HID_KEY_2,    HID_KEY_3,     HID_KEY_4,     HID_KEY_5,     HID_KEY_6,     HID_KEY_7,     HID_KEY_8     },
                { HID_KEY_TAB,       HID_KEY_Q,    HID_KEY_W,    HID_KEY_E,     HID_KEY_R,     HID_KEY_T,     HID_KEY_Y,     HID_KEY_U,     HID_KEY_I     },
                { HID_KEY_CAPS_LOCK, HID_KEY_A,    HID_KEY_S,    HID_KEY_D,     HID_KEY_F,     HID_KEY_G,     HID_KEY_H,     HID_KEY_J,     HID_KEY_K     },
                { HID_KEY_NONE,      HID_KEY_Z,    HID_KEY_X,    HID_KEY_C,     HID_KEY_V,     HID_KEY_B,     HID_KEY_N,     HID_KEY_M,     HID_KEY_COMMA },
                { HID_KEY_NONE,      HID_KEY_NONE, HID_KEY_NONE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_SPACE, HID_KEY_NONE  },
            },
            { // layer 1
                { HID_KEY_GRAVE, HID_KEY_F1,   HID_KEY_F2,   HID_KEY_F3,   HID_KEY_F4,   HID_KEY_F5,   HID_KEY_F6,   HID_KEY_F7,        HID_KEY_F8   },
                { HID_KEY_NONE,  HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE,      HID_KEY_NONE },
                { HID_KEY_NONE,  HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE,      HID_KEY_NONE },
                { HID_KEY_NONE,  HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE,      HID_KEY_NONE },
                { HID_KEY_NONE,  HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_BACKSPACE, HID_KEY_NONE },
            },
        },
        .modmap = {
            { // layer 0
                { 0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00,                        0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { KEYBOARD_MODIFIER_LEFTSHIFT, 0x00,                      0x00,                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { KEYBOARD_MODIFIER_LEFTCTRL,  KEYBOARD_MODIFIER_LEFTGUI, KEYBOARD_MODIFIER_LEFTALT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
            },
            { // layer 1
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
            },
        },
        .encmap = {
            { HID_KEY_NONE, HID_KEY_NONE }, // layer 0
            { HID_KEY_NONE, HID_KEY_NONE }, // layer 1
        },
    },
    .consumer = {
        .map = {
            { // layer 0
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
            },
            { // layer 1
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,                           0x0000,                        0x0000,                       0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,                           0x0000,                        0x0000,                       0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,                           0x0000,                        0x0000,                       0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,                           0x0000,                        0x0000,                       0x0000, 0x0000 },
                { 0x0000, 0x0000, 0x0000, 0x0000, HID_USAGE_CONSUMER_SCAN_PREVIOUS, HID_USAGE_CONSUMER_PLAY_PAUSE, HID_USAGE_CONSUMER_SCAN_NEXT, 0x0000, 0x0000 },
            },
        },
        .encmap = {
            { HID_USAGE_CONSUMER_VOLUME_DECREMENT, HID_USAGE_CONSUMER_VOLUME_INCREMENT }, // layer 0
            { 0x0000, 0x0000 }, // layer 1
        },
    },
};
