#ifndef ANIM_H
#define ANIM_H

#include <stdint.h>
#include <stdbool.h>

#define ANIM_STRIP_LEN (125)

typedef enum {
    ANIM_TYPE_OFF,
    ANIM_TYPE_EXTEND,
    ANIM_TYPE_RETRACT,
} anim_type_E;

void anim_init();
void anim_set(anim_type_E type, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale);
void anim_step();

#endif // ANIM_H
