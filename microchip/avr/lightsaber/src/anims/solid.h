#ifndef ANIMS_SOLID_H
#define ANIMS_SOLID_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
} solid_data_t;

void solid_reset(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale);
void solid_step(void *data);
void solid_get(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);
bool solid_done(void *data);

#endif // ANIMS_SOLID_H
