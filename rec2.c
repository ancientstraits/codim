#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "output.h"
#include "gfx.h"
#include "text.h"
#include "render.h"
#include "arecord.h"

#ifdef WIN32
#include <conio.h>
#else
#include <fcntl.h>
#endif

#define SECONDS 40

// Goals:
// 1. Use keypress to start/stop recording and play

// I made this video because I just wanted to upload something onto my youtube channel.
// This video uses the PortAudio library to record sound from a microphone.
// Then, it uses FFmpeg to put that sound into this video.
// For the last couple of days, I have been working on the Codim project.
// There may be another update soon, but there also may not be, if I stop working on Codim for whatever reason.

int main() {
    ARecordContext* arc = arecord_create(44100);
    arecord_open(arc);

    arecord_start(arc);
    puts("Recording...");
    size_t i = 0;
    clock_t time = clock();
    while (arecord_isactive(arc)) {
        clock_t c = clock();
        if ((c - time) >= CLOCKS_PER_SEC) {
            time = c;
            i++;
            printf("%llu seconds passed\n", i);
        }
        if (kbhit()) {
            arecord_stop(arc);
        }
    }
    arecord_close(arc);


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
        // if (frameno >= 24*SECONDS) {
        //     output_close(oc);
        // }
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
                *data++ = arc->udata.samples[sampleno];
                sampleno++;
            }
            if (sampleno >= arc->udata.idx*2)
                output_close(oc);

            output_encode_audio(oc);
        }
    }

    text_deinit(&tc);
    render_deinit(&rc);
    gfx_destroy(gc);
    output_destroy(oc);
    arecord_destroy(arc);
    return 0;
}
