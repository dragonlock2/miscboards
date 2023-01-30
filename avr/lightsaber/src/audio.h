#ifndef AUDIO_H
#define AUDIO_H

#define PWM_FREQ (25000) // Hz

typedef void (*audio_ticker_F)(void);

void audio_init(audio_ticker_F ticker);

#endif // AUDIO_H
