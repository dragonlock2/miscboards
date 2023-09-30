#ifndef RGB_H
#define RGB_H

typedef enum {
    RGB_NONE,
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE,
    RGB_COUNT,
} rgb_color_E;

void rgb_init();
void rgb(rgb_color_E color);

#endif // RGB_H
