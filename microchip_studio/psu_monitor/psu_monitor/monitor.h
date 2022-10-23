#ifndef MONITOR_H_
#define MONITOR_H_

typedef enum {
    MONITOR_CHAN_12V,
    MONITOR_CHAN_5V,
    MONITOR_CHAN_3V3,
    MONITOR_CHAN_N12V,
    MONITOR_CHAN_COUNT,
} monitor_chan_E;

void monitor_init();
int16_t monitor_read_mV(monitor_chan_E chan);
int16_t monitor_read_mA(monitor_chan_E chan);

#endif /* MONITOR_H_ */
