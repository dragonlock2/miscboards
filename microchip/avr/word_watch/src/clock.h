#pragma once

void clock_init(void);
void clock_now(uint8_t *hour, uint8_t *minute, uint8_t *second);
void clock_set(uint8_t hour, uint8_t minute, uint8_t second);
