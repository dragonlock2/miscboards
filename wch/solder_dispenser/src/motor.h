#pragma once
#include <stdint.h>

void motor_write(bool dir, uint8_t duty); // true => extrude
int32_t motor_read(void); // works with duty >=150
void motor_sleep(void);
void motor_wake(void);
