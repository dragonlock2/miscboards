#include "anims/off.h"
#include "leds.h"
#include "anim.h"

/* private defines */
typedef void (*anim_reset_F)(void *data, uint8_t r, uint8_t g, uint8_t b);
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
};

/* private data */
typedef struct {
    anim_type_E top_type;
    union {
        off_data_S off;
    } top_data;

    anim_type_E bot_type;
    union {
        off_data_S off;
    } bot_data;
} anim_data_S;
static anim_data_S anim_data;

/* public functions */
void anim_init() {
    leds_init();
    anim_set(true,  ANIM_TYPE_OFF, 0, 0, 0);
    anim_set(false, ANIM_TYPE_OFF, 0, 0, 0);
}

void anim_set(bool top, anim_type_E type, uint8_t r, uint8_t g, uint8_t b) {
    if (top) {
        anim_data.top_type = type;
        anim_cfgs[type].reset(&anim_data.top_data, r, g, b);
    } else {
        anim_data.bot_type = type;
        anim_cfgs[type].reset(&anim_data.bot_data, r, g, b);
    }
}

void anim_step() {
    uint8_t i, r, g, b;
    anim_cfgs[anim_data.top_type].step(&anim_data.top_data);
    for (i = 0; i < ANIM_TOP_STRIP_LEN; i++) {
        anim_cfgs[anim_data.top_type].get(&anim_data.top_data, i, &r, &g, &b);
        leds_write(true, r, g, b);
    }
    anim_cfgs[anim_data.bot_type].step(&anim_data.bot_data);
    for (i = 0; i < ANIM_BOT_STRIP_LEN; i++) {
        anim_cfgs[anim_data.bot_type].get(&anim_data.bot_data, i, &r, &g, &b);
        leds_write(false, r, g, b);
    }
}
