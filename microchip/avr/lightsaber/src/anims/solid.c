#include "solid.h"

void solid_reset(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale) {
    solid_data_t *d = (solid_data_t*) data;
    d->r = r;
    d->g = g;
    d->b = b;
}

void solid_step(void *data) {
}

void solid_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b) {
    solid_data_t *d = (solid_data_t*) data;
    *r = d->r;
    *g = d->g;
    *b = d->b;
}

bool solid_done(void *data) {
    return true;
}
