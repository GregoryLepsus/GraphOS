#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// Audio format structure
typedef struct {
    uint32_t sample_rate;    // 8000, 11025, 22050, 44100, 48000
    uint8_t channels;        // 1 = mono, 2 = stereo
    uint8_t bits_per_sample; // 8 or 16
    uint32_t buffer_size;
} audio_format_t;

// Audio device capabilities
typedef struct {
    uint32_t max_sample_rate;
    uint32_t min_sample_rate;
    uint8_t max_channels;
    uint8_t supports_16bit;
    uint32_t buffer_size;
} audio_caps_t;

// Audio states
#define AUDIO_STATE_STOPPED  0
#define AUDIO_STATE_PLAYING  1
#define AUDIO_STATE_PAUSED   2

// Standard sample rates
#define AUDIO_RATE_8000   8000
#define AUDIO_RATE_11025  11025
#define AUDIO_RATE_22050  22050
#define AUDIO_RATE_44100  44100
#define AUDIO_RATE_48000  48000

// Functions
void audio_init(void);
int audio_open(audio_format_t* format);
void audio_close(void);
int audio_play_buffer(void* buffer, uint32_t size);
int audio_write(void* data, uint32_t size);
void audio_set_volume(uint8_t volume);  // 0-100
uint8_t audio_get_volume(void);
void audio_pause(void);
void audio_resume(void);
void audio_stop(void);
int audio_get_state(void);
void audio_get_caps(audio_caps_t* caps);

#endif // AUDIO_H
