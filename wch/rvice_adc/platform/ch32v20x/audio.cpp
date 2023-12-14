#include <cstdbool>
#include <tusb.h>
#include "audio.h"

/* private data */
static constexpr size_t SAMPLES_PER_MS = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;

static struct {
    bool ping;
    int16_t buffer[2][SAMPLES_PER_MS];
} data;

/* public functions */
void audio_task(void* args) {
    (void) args;
    TickType_t wait = xTaskGetTickCount();
    while (true) {
        // TODO start next DMA transfer, remove this
        static int16_t x;
        for (size_t i = 0; i < SAMPLES_PER_MS; i++) {
            data.buffer[data.ping][i] = x++;
        }

        // push finished buffer to fifo
        tud_audio_write(reinterpret_cast<uint8_t*>(data.buffer[!data.ping]), sizeof(data.buffer[0]));

        // wait 1ms
        data.ping = !data.ping;
        vTaskDelayUntil(&wait, 1 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void audio_init(void) {
    xTaskCreate(audio_task, "audio_task", configMINIMAL_STACK_SIZE, NULL,
        configMAX_PRIORITIES - 2, NULL);
}
