#include <hardware/irq.h>
#include "config.h"
#include "kscan.h"
#include "ws2812b.pio.h"
#include "ws2812b.h"

#include <cstdio>

struct semaphore* ws2812b::dma_lut[NUM_DMA_CHANNELS];

ws2812b::ws2812b(uint rows, uint cols, const uint* led_pins)
:
    rows(rows), cols(cols), led_pins(led_pins)
{
    const PIO PIOS[NUM_PIOS] = { pio0, pio1 };
    const uint OFFSETS[NUM_PIOS] = {
        pio_add_program(pio0, &ws2812b_program),
        pio_add_program(pio1, &ws2812b_program),
    };
    for (uint i = 0; i < rows; i++) {
        pios[i] = PIOS[i / NUM_PIO_STATE_MACHINES];
        sms[i]  = pio_claim_unused_sm(pios[i], true);
        ws2812b_program_init(pios[i], sms[i], OFFSETS[i / NUM_PIO_STATE_MACHINES], LED_PINS[i]);

        sem_init(&done[i], 1, 1);
        dmas[i] = dma_claim_unused_channel(true);
        dma_cfgs[i] = dma_channel_get_default_config(dmas[i]);
        dma_lut[dmas[i]] = &done[i];
        channel_config_set_dreq(&dma_cfgs[i], pio_get_dreq(pios[i], sms[i], true));
        dma_channel_set_irq0_enabled(dmas[i], true);
    }
    irq_add_shared_handler(DMA_IRQ_0, dma_complete, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);
}

ws2812b_color& ws2812b::operator() (uint row, uint col) {
#if LED_COL_REVERSE
    return pixels[row][cols - 1 - col];
#else
    return pixels[row][col];
#endif
}

void ws2812b::display(void) {
    for (uint i = 0; i < rows; i++) {
        if (!sem_try_acquire(&done[i])) {
            panic("previous call to display didn't finish");
        }
        dma_channel_configure(dmas[i], &dma_cfgs[i], &pios[i]->txf[sms[i]], &pixels[i], cols, true);
    }
}

int64_t ws2812b::reset_complete(alarm_id_t id, void* dma_chan) {
    sem_release(dma_lut[(uint) dma_chan]);
    return 0;
}

void __isr ws2812b::dma_complete(void) {
    for (uint i = 0; i < NUM_DMA_CHANNELS; i++) {
        if (dma_lut[i] && (dma_hw->ints0 & (1 << i))) {
            dma_hw->ints0 = (1 << i);
            if (add_alarm_in_us(ws2812b_RESET, reset_complete, (void*) i, true) < 0) {
                panic("couldn't add reset timer callback");
            }
        }
    }
}
