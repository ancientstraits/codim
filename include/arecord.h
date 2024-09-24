#ifndef CODIM_ARECORD_H
#define CODIM_ARECORD_H

#define ARECORD_MAX_SECONDS 120

#include <stdint.h>
#include <portaudio.h>

typedef struct ARecordUserData {
    // `idx` is in number of frames
    // A frame could be 2 `int16_t`s if the audio is stereo
    size_t idx, max_frame;
    int16_t* samples;
} ARecordUserData;

typedef struct ARecordContext {
    int sample_rate;
    PaStreamParameters in_params;
    ARecordUserData udata;
    PaStream* stream;
} ARecordContext;

ARecordContext* arecord_create(int sample_rate);
void arecord_open(ARecordContext* arc);
void arecord_start(ARecordContext* arc);
int  arecord_isactive(ARecordContext* arc);
void arecord_stop(ARecordContext* arc);
void arecord_close(ARecordContext* arc);

// Print out `prompt`, then records.
void arecord_prompt_and_record(ARecordContext* arc, const char* prompt);

// be sure to `free` the `samples`
ARecordUserData arecord_get_data_copy(ARecordContext* arc);

void arecord_destroy(ARecordContext* arc);

#endif // !CODIM_ARECORD_H
