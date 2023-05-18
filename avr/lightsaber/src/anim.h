#ifndef ANIM_H
#define ANIM_H

#include <stdint.h>
#include <stdbool.h>

#define ANIM_STRIP_LEN (125)

typedef enum {
    ANIM_TYPE_SOLID,
    ANIM_TYPE_EXTEND,
    ANIM_TYPE_RETRACT,
} anim_type_t;

void anim_init();
void anim_set(anim_type_t type, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale, uint8_t speedup);
void anim_step();
bool anim_done();

#endif // ANIM_H
