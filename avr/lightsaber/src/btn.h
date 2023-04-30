#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

void btn_init();
void btn_run50Hz();
bool btn_rising();
bool btn_falling();

#endif // BTN_H
