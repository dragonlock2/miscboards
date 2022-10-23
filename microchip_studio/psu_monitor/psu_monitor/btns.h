#ifndef BTNS_H_
#define BTNS_H_

typedef enum {
    BTNS_CHANNEL_LEFT,
    BTNS_CHANNEL_RIGHT,
    BTNS_CHANNEL_CENTER,
    BTNS_CHANNEL_COUNT,
} btns_channel_E;

typedef enum {
    BTNS_PRESS_NONE,
    BTNS_PRESS_SHORT,
    BTNS_PRESS_LONG,
    BTNS_PRESS_HOLD,
} btns_press_E;

void btns_init();
void btns_loop();
btns_press_E btns_read(btns_channel_E chan);

#endif /* BTNS_H_ */
