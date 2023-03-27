#include "pio_alloc.h"

static PIO pios[NUM_PIOS] = { pio0, pio1 };

static bool alloc_list[NUM_PIOS][NUM_PIO_STATE_MACHINES];

bool pio_alloc(PIO &pio, uint &sm) {
    for (uint p = 0; p < NUM_PIOS; p++) {
        for (uint s = 0; s < NUM_PIO_STATE_MACHINES; s++) {
            if (!alloc_list[p][s]) {
                alloc_list[p][s] = true;
                pio = pios[p];
                sm  = s;
                return true;
            }
        }
    }
    return false;
}
