#include <string.h>
#include "extend.h"

void extend_reset(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale) {
    extend_data_t *d = (extend_data_t*) data;
    d->r = r;
    d->g = g;
    d->b = b;

    if (upscale > EXTEND_MAX_UPSCALE) {
        upscale = EXTEND_MAX_UPSCALE;
    }

    uint16_t overlap = upscale * EXTEND_OVERLAP;
    d->upscale    = upscale;
    d->window_len = upscale + 2 * overlap;
    d->super_idx  = 0;
    d->super_len  = upscale * ANIM_STRIP_LEN + 2 * overlap;
    memset(d->buckets, 0, sizeof d->buckets);

    for (uint16_t i = 0; i <= d->window_len; i++) {
        d->lut_r[i] = (uint16_t) r * i / d->window_len;
        d->lut_g[i] = (uint16_t) g * i / d->window_len;
        d->lut_b[i] = (uint16_t) b * i / d->window_len;
    }
}

void extend_step(void *data) {
    extend_data_t *d = (extend_data_t*) data;
    if (d->super_idx >= d->super_len) {
        d->done = true;
        return;
    }
    for (uint16_t i = 0; i < ANIM_STRIP_LEN; i++) {
        uint16_t start_idx = i * d->upscale;
        uint16_t end_idx   = start_idx + d->window_len - 1;
        if (d->super_idx >= start_idx && d->super_idx <= end_idx) {
            d->buckets[i]++;
        }
    }
    d->super_idx++;
}

void extend_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b) {
    extend_data_t *d = (extend_data_t*) data;
    *r = d->lut_r[d->buckets[idx]];
    *g = d->lut_g[d->buckets[idx]];
    *b = d->lut_b[d->buckets[idx]];
}

bool extend_done(void *data) {
    extend_data_t *d = (extend_data_t*) data;
    return d->done;
}

void retract_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b) {
    extend_data_t *d = (extend_data_t*) data;
    extend_get(data, ANIM_STRIP_LEN - idx - 1, r, g, b);
    *r = d->r - *r;
    *g = d->g - *g;
    *b = d->b - *b;
}
