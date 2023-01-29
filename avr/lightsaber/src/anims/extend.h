#ifndef ANIMS_EXTEND_H
#define ANIMS_EXTEND_H

#include <stdint.h>
#include <anim.h>

typedef struct {
    uint8_t  r, g, b;
    uint16_t len;
    uint16_t upscale, window_len;
    uint16_t super_idx, super_len;
    uint8_t  buckets[ANIM_MAX_STRIP_LEN];
} extend_data_S;

void extend_reset(extend_data_S *data, uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t upscale);
void extend_step(extend_data_S *data);
void extend_get(extend_data_S *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);
void retract_get(extend_data_S *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // ANIMS_EXTEND_H
