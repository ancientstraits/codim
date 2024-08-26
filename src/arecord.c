#include <stdlib.h>
#include <stdint.h>
#include <portaudio.h>

#ifdef WIN32
#include <conio.h>
#define SET_NONBLOCKING()
#define KEY_PRESSED() (kbhit())
#define SET_BLOCKING()
#else
#include <fcntl.h>
#define SET_NONBLOCKING() todo
#define KEY_PRESSED() (getchar() != EOF)
#define SET_BLOCKING() todo
#endif

#include "error.h"
#include "arecord.h"

#define AASSERT(cond, ...) ASSERT(cond, ERROR_ARECORD, __VA_ARGS__)

static int arecord_onrecord(
    const void* in, void* out,
    unsigned long frames_per_buf,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags flags,
    void* _udata
) {
    ARecordUserData* udata = _udata;
    const int16_t* read = in;
    int16_t* write = &udata->samples[2 * udata->idx];

    size_t frames_left = udata->max_frame - udata->idx;

    size_t frames_to_calculate;
    int ret;
    if (frames_left < frames_per_buf) {
        ret = paComplete;
        frames_to_calculate = frames_left;
    } else {
        ret = paContinue;
        frames_to_calculate = frames_per_buf;
    }

    if (read) {
        memcpy(write, read, frames_to_calculate * 2 * sizeof(int16_t));
    } else {
        memset(write, 0, frames_to_calculate * 2 * sizeof(int16_t));
    }

    udata->idx += frames_to_calculate;
    return ret;
}

ARecordContext* arecord_create(int sample_rate) {
    ARecordContext* arc = malloc(sizeof *arc);
    arc->sample_rate = sample_rate;

    arc->udata = (ARecordUserData){
        .max_frame = ARECORD_MAX_SECONDS * sample_rate,
        .idx = 0,
    };
    arc->udata.samples = malloc(arc->udata.max_frame * 2 * sizeof(int16_t));
    AASSERT(arc->udata.samples != NULL, "Could not allocate the samples");
    
    AASSERT(Pa_Initialize() == paNoError, "Could not initialize PortAudio");

    arc->in_params = (PaStreamParameters){
        .channelCount = 2,
        .sampleFormat = paInt16,
        .hostApiSpecificStreamInfo = NULL,
    };
    arc->in_params.device = Pa_GetDefaultInputDevice();
    AASSERT(arc->in_params.device != paNoDevice, "PortAudio could not find audio device");
    arc->in_params.suggestedLatency = Pa_GetDeviceInfo(arc->in_params.device)->defaultLowInputLatency;

    return arc;
}

void arecord_open(ARecordContext* arc) {
    AASSERT(
        Pa_OpenStream(&arc->stream, &arc->in_params, NULL, arc->sample_rate, 512, paClipOff, arecord_onrecord, &arc->udata)
            == paNoError,
    "Could not open PortAudio stream");
}

void arecord_start(ARecordContext* arc) {
    AASSERT(Pa_StartStream(arc->stream) == paNoError, "Could not start PortAudio stream");
}

int arecord_isactive(ARecordContext* arc) {
    return Pa_IsStreamActive(arc->stream);
}

void arecord_stop(ARecordContext* arc) {
    Pa_StopStream(arc->stream);
}

void arecord_close(ARecordContext* arc) {
    AASSERT(Pa_CloseStream(arc->stream) == paNoError, "Could not close PortAudio stream");

}

ARecordUserData arecord_get_data_copy(ARecordContext *arc) {
    ARecordUserData ret = { .idx = arc->udata.idx, .max_frame = arc->udata.max_frame };
    ret.samples = malloc(2*ret.idx*sizeof(int16_t));
    memcpy(ret.samples, arc->udata.samples, 2*ret.idx*sizeof(int16_t));
    return ret;
}

void arecord_reset(ARecordContext* arc) {
    memset(arc->udata.samples, 0, arc->udata.max_frame * 2 * sizeof(int16_t));
    arc->udata.idx = 0;
}

void arecord_destroy(ARecordContext* arc) {
    AASSERT(Pa_Terminate() == paNoError, "Could not terminate PortAudio");
    free(arc->udata.samples);
    free(arc);
}

void arecord_prompt_and_record(ARecordContext *arc, const char *prompt) {
    if (arc->udata.idx == 0)
        arecord_reset(arc);

    puts((prompt != NULL) ? prompt : "Recording started");
    SET_NONBLOCKING();

    arecord_open(arc);
    arecord_start(arc);

    while (arecord_isactive(arc)) {
        if (KEY_PRESSED())
            arecord_stop(arc);
    }

    SET_BLOCKING();
    puts("Recording ended");
    arecord_close(arc);
}
