#ifndef FONT_H
#define FONT_H

#define PWM_FREQ (25000) // Hz

typedef enum {
    FONT_TYPE_BOOT,
    FONT_TYPE_OFF,
    FONT_TYPE_ON,
    FONT_TYPE_SWING,
    FONT_TYPE_CLASH,
} font_type_t;

void font_init(void (*callback)(), uint16_t freq);
void font_sleep();
void font_wake();

void font_select(uint8_t i);
uint8_t font_num();

uint8_t font_r();
uint8_t font_g();
uint8_t font_b();
uint8_t font_upscale();
uint8_t font_speedup();

void font_play(font_type_t type);
bool font_done();

#endif // FONT_H
