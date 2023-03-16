#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <epoxy/gl.h>

#include "error.h"
#include "output.h"
#include "gfx.h"

#define WIDTH  600
#define HEIGHT 400

static void draw() {
	glClearColor(0.0, 0.5, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_POLYGON);
		glColor3f(1, 0, 0); glVertex3f(-0.6, -0.75, 0.5);
		glColor3f(0, 1, 0); glVertex3f(0.6, -0.75, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0.75, 0);
	glEnd();

	glFlush;
}

int main() {
	switch (ERROR_GET()) {
	case ERROR_NONE:
		break;
	default:
		LOG("Fatal error");
		return 1;
	}


	OutputContext* oc = output_create("out.mp4",
		&(OutputAudioOpts) {
			.sample_rate = 44100,
		}, &(OutputVideoOpts) {
			.fps = 24,
			.width = 1920,
			.height = 1080,
		}
	);

	output_open(oc);

	float at = 0;
	float atincr = 2 * M_PI * 110.0 / oc->acc->sample_rate;
	float atincr2 = atincr / oc->acc->sample_rate;

	GfxContext* gc = gfx_create(WIDTH, HEIGHT);

	while (output_is_open(oc)) {
		if (output_get_seconds(oc) >= 10.0)
			output_close(oc);

		if (output_get_encode_type(oc) == OUTPUT_TYPE_VIDEO) {
			draw();
			gfx_render(gc, oc->vf);

			output_encode_video(oc);
		} else {
			int16_t *data = (int16_t *)oc->afd->data[0];
			for (int i = 0; i < oc->afd->nb_samples; i++) {
				int v = sin(at) * 10000;
				for (int j = 0; j < oc->acc->ch_layout.nb_channels; j++)
					*data++ = v;
				at += atincr;
				atincr += atincr2;
			}
			output_encode_audio(oc);
		}
	}

	gfx_destroy(gc);
	output_destroy(oc);

	return 0;
}

