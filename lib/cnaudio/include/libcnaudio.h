#ifndef _LIBCNAUDIO_H_
    #define _LIBCNAUDIO_H_

    #include "libcncore.h"
    #include <SDL_mixer.h>

    typedef enum {
        CNAUDIO_EVENT_NOOP = 0x00,
        CNAUDIO_EVENT_PLAY = 0x01,
        CNAUDIO_EVENT_PAUSE = 0x02,
        ECNAUDIO_VENT_RESTART = 0x03,
        CNAUDIO_EVENT_VOLUME = 0x04, // value=(volume: int)
        CNAUDIO_EVENT_SEQ_END = 0x05, // value=(force: bool)
        CNAUDIO_EVENT_PANNING = 0x06  // value=((left: int,right: int))
    } audio_event;

    struct audio_event_s {
        audio_event type;
        cn_value value;

        cntime start_in; // ms
    };

    struct audio_sequence_s {
        audio_event *event_list;
        size_t size;
        size_t capacity;

        cntime sequence_timer;
    };

    struct audio_s {
        const Mix_Chunk *audio_ptr;
        struct audio_sequence_s sequence;

        int32_t playing_on;

        cnbool is_playing;
        cnbool is_ended;

        void *on_end;
    };

    typedef struct audio_event_s AudioEvent;
    typedef struct audio_sequence_s AudioSequence;
    typedef struct audio_s Audio;

    CN_API Audio *new_audio(const Mix_Chunk *audio_chunk_ptr, cn_method on_audio_end);
    CN_API void audio_run_sequence(Audio *audio, int32_t delta_time, int32_t *free_channels, size_t number_of_free_channel);
    CN_API void play_audio(Audio *audio, int32_t channel);
    CN_API void set_volume_audio(Audio *audio, int32_t volume);
    CN_API void set_panning_audio(Audio *audio, uint8_t left, uint8_t right);
    CN_API void stop_audio(Audio *audio);
    CN_API void delete_audio(Audio *audio);

#endif
