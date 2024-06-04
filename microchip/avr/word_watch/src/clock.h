#pragma once

void clock_init(void);
uint16_t clock_ticks(void); // careful overflow
void clock_now(int8_t *hour, int8_t *minute, int8_t *second);
void clock_set(int8_t hour, int8_t minute, int8_t second);
