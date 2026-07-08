#include "libcnaudio.h"

CN_API void init_audio_sequence_content(AudioSequence *seq)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't init empty audio sequence.");
        return;
    }

    seq->capacity = 0;
    seq->size = 0;
    seq->event_list = NULL;
    seq->sequence_timer = 0;
}

CN_API void audio_sequence_increment(AudioSequence *seq, double delta_time)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't increment empty audio sequence.");
        return;
    }

    seq->sequence_timer += delta_time;
}

CN_API void audio_sequence_push_event(AudioSequence *seq, const AudioEvent *ev)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't push event in empty audio sequence.");
        return;
    }
    
    if (!ev) {
        RAISE(ERR_INVALID_POINTER, "can't push empty event in audio sequence.");
        return;
    }

    // todo: implement
}

CN_API void audio_sequence_add_event(AudioSequence *seq, audio_event event_type, cn_value value, double raw_delta_since_start)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't add event in empty audio sequence.");
        return;
    }

    (void)event_type;
    (void)value;
    (void)raw_delta_since_start;
    // todo: implement
}


CN_API cnbool audio_sequence_pull_event(const AudioSequence *seq, AudioEvent *ev)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't pull event in empty audio sequence.");
        return (false);
    }
    
    if (!ev) {
        RAISE(ERR_INVALID_POINTER, "can't pull empty event in audio sequence.");
        return (false);
    }

    // todo: implement
    return (false);
}

CN_API cnbool audio_sequence_is_empty(const AudioSequence *seq)
{
    return (!seq->event_list || !seq->size);
}

CN_API void delete_audio_sequence_content(AudioSequence *seq)
{
    if (!seq) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty audio sequence.");
        return;
    }
    if (seq->event_list)
        (void)free(seq->event_list);
    seq->size = 0;
    seq->capacity = 0;
    seq->sequence_timer = 0;
    seq->event_list = NULL;
}
