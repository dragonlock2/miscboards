#ifndef ANIMS_OFF_H
#define ANIMS_OFF_H

#include <stdint.h>

typedef struct {
} off_data_S;

void off_reset(off_data_S *data, uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t upscale);
void off_step(off_data_S *data);
void off_get(off_data_S *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // ANIMS_OFF_H
