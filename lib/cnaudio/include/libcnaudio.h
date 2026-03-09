#ifndef _LIBCNAUDIO_H_
    #define _LIBCNAUDIO_H_

    #include "libcncore.h"
    #include <SDL2/SDL_mixer.h>

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

        cntime start_in;
    };

    struct audio_sequence_s {
        audio_event *event_list;

        cntime sequence_timer;
    };

    struct audio_s {
        const Mix_Chunk *audio_ptr;
        struct audio_sequence_s sequence;

        int32_t paying_on;

        cnbool is_playing;
        cnbool is_ended;

        void *on_end;
    };

    typedef struct audio_event_s AudioEvent;
    typedef struct audio_sequence_s AudioSequence;
    typedef struct audio_s Audio;

#endif
