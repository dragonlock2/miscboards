#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>

#define ANIM_STRIP_LEN (14)

typedef enum {
    ANIM_TYPE_OFF,
    ANIM_TYPE_EXTEND,
    ANIM_TYPE_RETRACT,
} anim_type_E;

void anim_init();
void anim_set(bool top, anim_type_E type);
void anim_step();

#endif // ANIM_H
