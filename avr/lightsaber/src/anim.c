#include "anims/off.h"
#include "anims/extend.h"
#include "leds.h"
#include "anim.h"

/* private defines */
typedef void (*anim_reset_F)(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t upscale);
typedef void (*anim_step_F)(void *data);
typedef void (*anim_get_F)(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);

typedef struct {
    anim_reset_F reset;
    anim_step_F  step;
    anim_get_F   get;
} anim_cfg_S;

const anim_cfg_S anim_cfgs[] = {
    [ANIM_TYPE_OFF] = {
        .reset = (anim_reset_F) off_reset,
        .step = (anim_step_F) off_step,
        .get = (anim_get_F) off_get,
    },
    [ANIM_TYPE_EXTEND] = {
        .reset = (anim_reset_F) extend_reset,
        .step = (anim_step_F) extend_step,
        .get = (anim_get_F) extend_get,
    },
    [ANIM_TYPE_RETRACT] = {
        .reset = (anim_reset_F) extend_reset,
        .step = (anim_step_F) extend_step,
        .get = (anim_get_F) retract_get,
    }
};

/* private data */
typedef struct {
    anim_type_E type;
    union {
        off_data_S off;
        extend_data_S extend;
    } data;
} anim_data_S;
static anim_data_S anim_data;

/* public functions */
void anim_init() {
    leds_init();
    anim_set(ANIM_TYPE_OFF, 0, 0, 0, 0);
}

void anim_set(anim_type_E type, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale) {
    anim_data.type = type;
    anim_cfgs[type].reset(&anim_data.data, r, g, b, ANIM_STRIP_LEN, upscale);
}

void anim_step() {
    uint8_t i, r, g, b;
    for (i = 0; i < ANIM_STRIP_LEN; i++) {
        anim_cfgs[anim_data.type].get(&anim_data.data, i, &r, &g, &b);
        leds_write(r, g, b);
    }
    anim_cfgs[anim_data.type].step(&anim_data.data);
}
