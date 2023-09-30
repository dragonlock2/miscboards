#ifndef ANIMS_EXTEND_H
#define ANIMS_EXTEND_H

#include <stdbool.h>
#include <stdint.h>
#include <anim.h>

#define EXTEND_OVERLAP     (4)  // how many LEDs on each side to average with
#define EXTEND_MAX_UPSCALE (3)  // largest supported upscale amount

#define EXTEND_LUT_LEN (EXTEND_MAX_UPSCALE * (1 + 2 * EXTEND_OVERLAP) + 1)

typedef struct {
    uint8_t  r, g, b;
    uint16_t upscale, window_len;
    uint16_t super_idx, super_len;
    bool     done;
    uint8_t  buckets[ANIM_STRIP_LEN];
    uint8_t  lut_r[EXTEND_LUT_LEN];
    uint8_t  lut_g[EXTEND_LUT_LEN];
    uint8_t  lut_b[EXTEND_LUT_LEN];
} extend_data_t;

void extend_reset(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale);
void extend_step(void *data);
void extend_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);
bool extend_done(void *data);

void retract_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // ANIMS_EXTEND_H
