#include "encoder.h"

encoder::encoder(const kb_config& cfg)
:
    state(DETENT), reverse(cfg.enc.rev)
{
    // find available PIO state machine
    int s;
    if ((s = pio_claim_unused_sm(pio0, false)) >= 0) {
        pio = pio0;
        sm = s;
    } else {
        pio = pio1;
        sm = pio_claim_unused_sm(pio1, true);
    }

    // install/start program
    uint offset = pio_add_program(pio0, &encoder_program);
    encoder_program_init(pio, sm, offset, cfg.enc.a, cfg.enc.b);
}

encoder::operator int() {
    int ticks = encoder_process(pio, sm, state);
    return reverse ? ticks : -ticks;
}
