#include "libcnaudio.h"

CN_API void init_audio_sequence_content(AudioSequence *seq)
{
    seq->capacity = 0;
    seq->size = 0;
    seq->event_list = NULL;
    seq->sequence_timer = 0;
}

CN_API void audio_sequence_increment(AudioSequence *seq, double delta_time)
{
    seq->sequence_timer += delta_time;
}

CN_API void audio_sequence_push_event(AudioSequence *seq, const AudioEvent *ev)
{
    (void)seq;
    (void)ev;
    // todo: implement
}

CN_API void audio_sequence_add_event(AudioSequence *seq, audio_event event_type, cn_value value, double raw_delta_since_start)
{
    (void)seq;
    (void)event_type;
    (void)value;
    (void)raw_delta_since_start;
    // todo: implement
}


CN_API cnbool audio_sequence_pull_event(const AudioSequence *seq, AudioEvent *ev)
{
    (void)seq;
    (void)ev;

    // todo: implement
    return (false);
}

CN_API cnbool audio_sequence_is_empty(const AudioSequence *seq)
{
    return (!seq->event_list || !seq->size);
}

CN_API void delete_audio_sequence_content(AudioSequence *seq)
{
    if (seq->event_list)
        (void)free(seq->event_list);
    seq->size = 0;
    seq->capacity = 0;
    seq->sequence_timer = 0;
    seq->event_list = NULL;
}
