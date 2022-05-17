#include <stdio.h>
#include <math.h>
#include "output.h"

int main() {
    OutputContext* oc = output_context_create("out.mp4",
    &(OutputAudioOpts){
        .sample_rate = 44100,
    }, &(OutputVideoOpts){
        .fps = 24,
        .width = 1920,
        .height = 1080,
    });

    output_context_open(oc);

	float at = 0;
	float atincr = 2 * M_PI * 110.0 / oc->acc->sample_rate;
	float atincr2 = atincr / oc->acc->sample_rate;

    while (output_context_is_open(oc)) {
        if (output_context_get_seconds(oc) >= 10.0)
            output_context_close(oc);

        if (output_context_get_encode_type(oc) == OUTPUT_TYPE_VIDEO) {
            for (int i = 0; i < oc->vf->height; i++) {
                for (int j = 0; j < oc->vf->width; j++) {
                    oc->vf->data[0][(i * oc->vf->linesize[0]) + j] = 0;
                }
            }
            for (int i = 0; i < oc->vf->height / 2; i++) {
                for (int j = 0; j < oc->vf->width / 2; j++) {
                    oc->vf->data[1][(i * oc->vf->linesize[1]) + j] = (j + oc->vpts) % 256;
                    oc->vf->data[2][(i * oc->vf->linesize[2]) + j] = (i + oc->vpts) % 256;
                }
            }

            output_context_encode_video(oc);
        } else {
            int16_t *data = (int16_t *)oc->afd->data[0];
            for (int i = 0; i < oc->afd->nb_samples; i++) {
                int v = sin(at) * 10000;
                for (int j = 0; j < oc->acc->channels; j++)
                    *data++ = v;
                at += atincr;
                atincr += atincr2;
            }
            output_context_encode_audio(oc);
        }
    }

    output_context_destroy(oc);

    return 0;
}
