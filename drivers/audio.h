#ifndef AUDIO_H
#define AUDIO_H
#include "../kernel/types.h"
#define AUDIO_BUFFER_SIZE 4096
typedef struct {
    bool initialized, playing;
    uint32_t sample_rate;
    uint8_t channels, bits, volume;
    uint8_t buffer[AUDIO_BUFFER_SIZE];
    uint32_t buf_head, buf_tail;
} audio_t;
bool audio_init(void);
void audio_play(void);
void audio_stop(void);
void audio_set_volume(uint8_t vol);
void audio_beep(uint32_t freq, uint32_t ms);
bool audio_write(const uint8_t* data, uint32_t size);
#endif
