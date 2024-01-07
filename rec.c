#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "output.h"
#include "gfx.h"
#include "text.h"
#include "render.h"
#include "portaudio.h"

#define SECONDS 40

// Goals:
// 1. Use keypress to start/stop recording and play

// I made this video because I just wanted to upload something onto my youtube channel.
// This video uses the PortAudio library to record sound from a microphone.
// Then, it uses FFmpeg to put that sound into this video.
// For the last couple of days, I have been working on the Codim project.
// There may be another update soon, but there also may not be, if I stop working on Codim for whatever reason.

typedef struct {
    int idx;
    int maxFrame;
    uint16_t* samples;
} UData;

static int on_record(
    const void* in, void* out,
    unsigned long frames_per_buf,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags flags,
    void* _udata
) {
    UData* udata = _udata;
    const uint16_t* read = in;
    uint16_t* write = &udata->samples[udata->idx * 2];

    unsigned long frames_left = udata->maxFrame - udata->idx;
    unsigned long frames_to_calculate;
    int ret;
    if (frames_left < frames_per_buf) {
        ret = paComplete;
        frames_to_calculate = frames_left;
    } else {
        ret = paContinue;
        frames_to_calculate = frames_per_buf;
    }

    if (read) {
        for (long i = 0; i < 2*frames_to_calculate; i++) {
            write[i] = read[i];
        }
    } else {
        for (long i = 0; i < 2*frames_to_calculate; i++) {
            write[i] = 0.0;
        }
    }

    udata->idx += frames_to_calculate;
    return ret;
}

int main() {
    int err;

    UData udata = {
        .maxFrame = SECONDS * 44100,
        .idx      = 0,
    };
    udata.samples = malloc(udata.maxFrame * 2 * sizeof(int16_t));
    assert(udata.samples);

    assert(!Pa_Initialize());
    
    PaStreamParameters inParams;
    inParams.device = Pa_GetDefaultInputDevice();
    assert(inParams.device != paNoDevice);
    inParams.channelCount = 2; // stereo
    inParams.sampleFormat = paInt16;
    inParams.suggestedLatency = Pa_GetDeviceInfo(inParams.device)->defaultLowInputLatency;
    // TODO can we do something with this?
    inParams.hostApiSpecificStreamInfo = NULL;

    PaStream* stream;
    assert(!Pa_OpenStream(
        &stream, &inParams, NULL, 44100, 512, paClipOff, on_record, &udata
    ));
    assert(!Pa_StartStream(stream));
    puts("Recording...");
    int i = 1;
    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(1000);
        printf("%d seconds\n", i++);
    }
    assert(err == 0);
    assert(!Pa_CloseStream(stream));

    printf("%d\n", udata.idx);

    Pa_Terminate();

    size_t sampleno = 0, frameno = 0;
    OutputContext* oc = output_create("myfile.mp4", &(OutputAudioOpts){
        .sample_rate = 44100,
    }, &(OutputVideoOpts) {
        .width = 1920,
        .height = 1080,
        .fps = 24,
    });
    GfxContext* gc = gfx_create(1920, 1080);
    RenderContext rc;
    render_init(&rc);
    TextContext tc;
    text_init(&tc, "sample.ttf", 45);
    RenderDrawable rd;
    text_render(&tc,
        "I made this video because I just wanted to upload something onto my youtube channel.\n"
        "This video uses the PortAudio library to record sound from a microphone.\n"
        "Then, it uses FFmpeg to put that sound into this video.\n"
        "For the last couple of days, I have been working on the Codim project.\n"
        "There may be another update soon, but there also may not be,\n"
        "if I stop working on Codim for whatever reason."
        "\n\n\n\n-- AncientStraits 2024"
    , 60.0, 1000.0, &rd);
    render_add(&rc, rd);

    output_open(oc);
    while (output_is_open(oc)) {
        if (frameno >= 24*SECONDS) {
            output_close(oc);
        }
        if (output_get_encode_type(oc) == OUTPUT_TYPE_VIDEO) {
            glClearColor(0.3, 0.5, 0.7, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            render(&rc, 1920, 1080);
            gfx_render(gc, oc->vf);
            output_encode_video(oc);
            frameno++;
        } else if (output_get_encode_type(oc) == OUTPUT_TYPE_AUDIO) {
            assert(oc->acc->ch_layout.nb_channels == 2);
            int16_t* data = (int16_t*)oc->afd->data[0];
            for (int i = 0; i < oc->afd->nb_samples * 2; i++) {
                *data++ = udata.samples[sampleno];
                if (sampleno <= udata.maxFrame*2) sampleno++;
            }

            output_encode_audio(oc);
        }
    }

    text_deinit(&tc);
    render_deinit(&rc);
    gfx_destroy(gc);
    output_destroy(oc);
    free(udata.samples);
    return 0;
}
