#include <string.h>
#include "extend.h"

#define EXTEND_OVERLAP (4) // how many LEDs on each side to average with

void extend_reset(extend_data_S *data, uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t upscale) {
    data->r = r;
    data->g = g;
    data->b = b;
    data->len = len;

    uint16_t overlap = upscale * EXTEND_OVERLAP;
    data->upscale    = upscale;
    data->window_len = upscale + 2 * overlap;
    data->super_idx  = 0;
    data->super_len  = upscale * len + 2 * overlap;
    memset(data->buckets, 0, sizeof data->buckets);

    if (data->window_len > EXTEND_LUT_LEN) {
        data->window_len = EXTEND_LUT_LEN;
    }
    for (uint16_t i = 0; i <= data->window_len; i++) {
        data->lut_r[i] = (uint16_t) r * i / data->window_len;
        data->lut_g[i] = (uint16_t) g * i / data->window_len;
        data->lut_b[i] = (uint16_t) b * i / data->window_len;
    }
}

void extend_step(extend_data_S *data) {
    if (data->super_idx >= data->super_len) {
        return;
    }
    for (uint16_t i = 0; i < data->len; i++) {
        uint16_t start_idx = i * data->upscale;
        uint16_t end_idx   = start_idx + data->window_len - 1;
        if (data->super_idx >= start_idx && data->super_idx <= end_idx) {
            data->buckets[i]++;
        }
    }
    data->super_idx++;
}

void extend_get(extend_data_S *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = data->lut_r[data->buckets[idx]];
    *g = data->lut_g[data->buckets[idx]];
    *b = data->lut_b[data->buckets[idx]];
}

void retract_get(extend_data_S *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b) {
    extend_get(data, data->len - idx - 1, r, g, b);
    *r = data->r - *r;
    *g = data->g - *g;
    *b = data->b - *b;
}
