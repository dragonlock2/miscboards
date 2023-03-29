#ifndef ENCODER_H
#define ENCODER_H

#include <pico/stdlib.h>

enum class encoder_state {
    DETENT,
    CW1,  CW2,  CW3,
    CCW1, CCW2, CCW3,
    COUNT
};

class encoder {
public:
    encoder(uint a, uint b);

    void scan(void);
    operator int();

private:
    encoder_state state;
    const uint a, b;
    int ticks;
};

#endif // ENCODER_H
