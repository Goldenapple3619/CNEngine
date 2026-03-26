#include "libcnaudio.h"

CN_API Audio *new_audio(const Mix_Chunk *audio_chunk_ptr,
    cn_method on_audio_end)
{
    Audio *audio = (Audio *)malloc(sizeof(Audio));

    if (!audio)
        return (NULL);
    audio->audio_ptr = audio_chunk_ptr;
    audio->sequence.event_list = NULL;
    audio->sequence.size = 0;
    audio->sequence.capacity = 0;
    audio->sequence.sequence_timer = 0;
    audio->on_end = on_audio_end;
    audio->playing_on = -1;
    audio->is_ended = false;
    audio->is_playing = false;
    return (audio);
}

CN_API void audio_run_sequence(Audio *audio, int32_t delta_time, int32_t *free_channels, size_t number_of_free_channel)
{
    if (!audio || !free_channels || number_of_free_channel == 0)
        return;

    (void)delta_time;
    //to implement
}

CN_API void play_audio(Audio *audio, int32_t channel)
{
    if (audio)
        return;

    if (audio->is_playing)
        (void)stop_audio(audio);
    audio->playing_on = Mix_PlayChannel(channel, (Mix_Chunk *)audio->audio_ptr, 0);
    
    if (audio->playing_on != -1)
        audio->is_playing = true;
}

CN_API void set_volume_audio(Audio *audio, int32_t volume)
{
    if (!audio->is_ended || audio->playing_on == -1)
        return;

    Mix_Volume(audio->playing_on, volume);
}

CN_API void set_panning_audio(Audio *audio, uint8_t left, uint8_t right)
{
    if (!audio->is_ended || audio->playing_on == -1)
        return;

    Mix_SetPanning(audio->playing_on, left, right);
}

CN_API void stop_audio(Audio *audio)
{
    if (!audio || !audio->is_playing || audio->playing_on == -1)
        return;
    (void)Mix_HaltChannel(audio->playing_on);
    audio->playing_on = -1;
    audio->is_playing = false;
}

CN_API void delete_audio(Audio *audio)
{
    if (!audio)
        return;
    if (audio->sequence.event_list)
        (void)free(audio->sequence.event_list);
    (void)stop_audio(audio);
    audio->is_ended = true;
    (void)free(audio);
}
