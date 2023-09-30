#include "anims/solid.h"
#include "anims/extend.h"
#include "leds.h"
#include "anim.h"

/* private defines */
typedef void (*anim_reset_t)(void *data, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale);
typedef void (*anim_step_t)(void *data);
typedef void (*anim_get_t)(void *data, uint8_t idx, uint8_t *r, uint8_t *g, uint8_t *b);
typedef bool (*anim_done_t)(void *data);

typedef struct {
    anim_reset_t reset;
    anim_step_t  step;
    anim_get_t   get;
    anim_done_t  done;
} anim_cfg_t;

static const anim_cfg_t anim_cfgs[] = {
    [ANIM_TYPE_SOLID] = {
        .reset = solid_reset,
        .step  = solid_step,
        .get   = solid_get,
        .done  = solid_done,
    },
    [ANIM_TYPE_EXTEND] = {
        .reset = extend_reset,
        .step  = extend_step,
        .get   = extend_get,
        .done  = extend_done,
    },
    [ANIM_TYPE_RETRACT] = {
        .reset = extend_reset,
        .step  = extend_step,
        .get   = retract_get,
        .done  = extend_done,
    },
};

/* private data */
static struct {
    anim_type_t type;
    uint8_t speedup;
    union {
        solid_data_t  solid;
        extend_data_t extend;
    } data;
} anim_data;

/* public functions */
void anim_init() {
    leds_init();
    anim_set(ANIM_TYPE_SOLID, 0, 0, 0, 1, 1);
}

void anim_set(anim_type_t type, uint8_t r, uint8_t g, uint8_t b, uint8_t upscale, uint8_t speedup) {
    anim_data.type = type;
    anim_data.speedup = speedup;
    anim_cfgs[type].reset(&anim_data.data, r, g, b, upscale);
}

void anim_step() {
    uint8_t r, g, b;
    for (uint8_t i = 0; i < ANIM_STRIP_LEN; i++) {
        anim_cfgs[anim_data.type].get(&anim_data.data, i, &r, &g, &b);
        leds_write(r, g, b);
    }
    for (uint8_t i = 0; i < anim_data.speedup; i++) {
        anim_cfgs[anim_data.type].step(&anim_data.data);
    }
}

bool anim_done() {
    return anim_cfgs[anim_data.type].done(&anim_data.data);
}
