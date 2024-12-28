#ifndef ENCODER_H
#define ENCODER_H

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include "config.h"
#include "encoder.pio.h"

class encoder {
public:
    encoder(const kb_config& cfg);

    operator int();

private:
    void process(void);

    PIO pio;
    uint sm;
    encoder_state state;
    bool reverse;
};

#endif // ENCODER_H
