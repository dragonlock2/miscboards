#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

void btn_init();
void btn_sleep();
void btn_wake();
void btn_run();

bool btn_pressed();
bool btn_hold();
bool btn_short_pressed();

#endif // BTN_H
