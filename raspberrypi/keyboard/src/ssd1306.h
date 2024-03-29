#ifndef SSD1306_H
#define SSD1306_H

#include <hardware/dma.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <pico/sem.h>
#include "config.h"

class ssd1306 {
public:
    ssd1306(const kb_config& cfg);

    void set_cursor(uint x, uint y);
    void draw_pixel(uint x, uint y, bool on);
    void draw_string(const char *s);
    void draw_char(char c);
    void clear(void);
    bool display(void); // true if prior frame done, starts next one

    const uint height;
    const uint width;
private:
    void send_command(uint8_t cmd);

    i2c_inst_t *const i2c;
    const uint8_t addr;

    uint x;
    uint y;
    uint8_t pixels[MAX_SSD1306_HEIGHT * MAX_SSD1306_WIDTH / 8];
    uint16_t dma_buf[MAX_SSD1306_HEIGHT * MAX_SSD1306_WIDTH / 8 + 1]; // one extra for register
    uint dma;
    dma_channel_config dma_cfg;
    struct semaphore done;

    static struct semaphore* dma_lut[NUM_DMA_CHANNELS];
    static int64_t reset_complete(alarm_id_t id, void* dma_chan);
    static void __isr dma_complete(void);
};

#endif // SSD1306_H
