#include "encoder.h"

encoder::encoder(uint a, uint b, bool reverse)
:
    state(DETENT), reverse(reverse)
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
    encoder_program_init(pio, sm, offset, a, b);
}

encoder::operator int() {
    int ticks = encoder_process(pio, sm, state);
    return reverse ? ticks : -ticks;
}
