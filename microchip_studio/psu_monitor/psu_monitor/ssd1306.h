#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdbool.h>
#include <string.h>

void ssd1306_init();
void ssd1306_set_cursor(uint16_t x, uint16_t y);
void ssd1306_draw_string(const char *s);
void ssd1306_draw_char(char c);
void ssd1306_draw_pixel(uint16_t x, uint16_t y, bool on);
void ssd1306_clear_display();
void ssd1306_display();

#endif /* SSD1306_H_ */
