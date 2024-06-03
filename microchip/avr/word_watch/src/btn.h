#pragma once

typedef struct {
    bool up, center, down;
    bool shake;
} btn_t;

void btn_init(void);
void btn_read(btn_t *status);
bool btn_waked(void);
void btn_sleep(void);
void btn_wake(void);
