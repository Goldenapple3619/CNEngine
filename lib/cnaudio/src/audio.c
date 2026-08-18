#include "libcnaudio.h"
#include <SDL_mixer.h>

CN_API Audio *new_audio(const Mix_Chunk *audio_chunk_ptr,
    cn_method on_audio_end, Object *on_end_obj)
{
    Audio *audio = (Audio *)malloc(sizeof(Audio));

    if (!audio) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new audio.");
        return (NULL);
    }
    audio->audio_ptr = audio_chunk_ptr;
    audio->on_end = on_audio_end;
    audio->on_end_obj = on_end_obj;
    audio->playing_on = -1;
    audio->is_ended = false;
    audio->is_playing = false;
    init_audio_sequence_content(&audio->sequence);
    return (audio);
}

CN_API void audio_run_sequence(Audio *audio, int32_t delta_time, struct free_channels_arr_s *free_channels)
{
    if (!audio) {
        RAISE(ERR_INVALID_POINTER, "can't run sequence from empty audio.");
        return;
    }

    AudioEvent end_seq = {.type = CNAUDIO_EVENT_NOOP};
    AudioEvent ev = {0};

    if (audio->is_playing && audio->playing_on != -1) {
        if (Mix_Playing(audio->playing_on) == 0) {
            audio->is_playing = false;
            audio->playing_on = -1;

            audio->on_end(audio->on_end_obj, (cnany[]){audio});
        }
    }

    audio_sequence_increment(&audio->sequence, delta_time);

    while (audio_sequence_pull_event(&audio->sequence, &ev)) {
        switch (ev.type) {
            case CNAUDIO_EVENT_NOOP:
                break;
            
            case CNAUDIO_EVENT_PLAY:
                if (!free_channels || free_channels->size == 0 || !free_channels->free_channels_arr) {
                    continue;
                    audio_sequence_push_event(&audio->sequence, &ev);
                }
                play_audio(audio, free_channels->free_channels_arr[(free_channels->size) - 1]);
                free_channels->size = free_channels->size - 1;
                break;
            case CNAUDIO_EVENT_SEQ_END:
                end_seq = ev;
                break;
            case CNAUDIO_EVENT_VOLUME:
                set_volume_audio(audio, ev.value.as.num);
                break;
            case CNAUDIO_EVENT_PANNING:
                set_panning_audio(audio, ev.value.as.vec2.x, ev.value.as.vec2.y);
                break;
            default:
                break;
        }
    }

    if (end_seq.type == CNAUDIO_EVENT_NOOP)
        return;
        
    if (end_seq.value.as.b || audio_sequence_is_empty(&audio->sequence)) {
        audio->is_ended = true;
        return;
    }

    audio_sequence_push_event(&audio->sequence, &end_seq);

}

CN_API void play_audio(Audio *audio, int32_t channel)
{
    if (audio) {
        RAISE(ERR_INVALID_POINTER, "can't play empty audio.");
        return;
    }

    if (audio->is_playing)
        (void)stop_audio(audio);
    audio->playing_on = Mix_PlayChannel(channel, (Mix_Chunk *)audio->audio_ptr, 0);
    
    if (audio->playing_on != -1)
        audio->is_playing = true;
}

CN_API void set_volume_audio(Audio *audio, int32_t volume)
{
    if (!audio) {
        RAISE(ERR_INVALID_POINTER, "can't set volume on empty audio.");
        return;
    }

    if (!audio->is_ended || audio->playing_on == -1)
        return;

    Mix_Volume(audio->playing_on, volume);
}

CN_API void set_panning_audio(Audio *audio, uint8_t left, uint8_t right)
{
    if (!audio) {
        RAISE(ERR_INVALID_POINTER, "can't set panning on empty audio.");
        return;
    }

    if (!audio->is_ended || audio->playing_on == -1)
        return;

    Mix_SetPanning(audio->playing_on, left, right);
}

CN_API void stop_audio(Audio *audio)
{
    if (!audio) {
        RAISE(ERR_INVALID_POINTER, "can't stop empty audio.");
        return;
    }

    if (!audio->is_playing || audio->playing_on == -1) {
        return;
    }

    (void)Mix_HaltChannel(audio->playing_on);
    audio->playing_on = -1;
    audio->is_playing = false;
}

CN_API void delete_audio(Audio *audio)
{
    if (!audio) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty audio.");
        return;
    }
    delete_audio_sequence_content(&audio->sequence);
    (void)stop_audio(audio);
    audio->is_ended = true;
    (void)free(audio);
}
