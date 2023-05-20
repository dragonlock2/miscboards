#ifndef LEDS_H
#define LEDS_H

#include <stdbool.h>
#include <stdint.h>

void leds_init();
void leds_sleep();
void leds_wake();
void leds_write(uint8_t r, uint8_t g, uint8_t b);

#endif // LEDS_H
