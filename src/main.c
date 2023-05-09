#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <epoxy/gl.h>

#include "error.h"
#include "output.h"
#include "glutil.h"
#include "gfx.h"
#include "text.h"
#include "render.h"
#include "scripting.h"

#define WIDTH  600
#define HEIGHT 400

static const char* vert_source =
	"#version 410 core\n"
	"layout (location = 0) in vec2 coord;"
	"layout (location = 1) in float intensity;"
	//"uniform vec2 dimension;"
	"layout (location = 2) in vec2 dimension;"
	"out float intens;"

	"vec2 ndc(vec2 coord, vec2 dim) {"
		"return (2.0 * (coord + 0.5) / dim) - 1.0;"
	"}"

	"void main() {"
		//"vec2 fcoord = vec2(ndc(coord.x, 600.0), ndc(coord.y, 400.0));"
		"vec2 fcoord = ndc(coord, dimension);"
		"gl_Position = vec4(fcoord.x, -fcoord.y, 0.0, 1.0);"
		"intens = intensity;"
	"}"
;
static const char* frag_source = 
	"#version 410 core\n"
	"in float intens;"
	"out vec4 color;"
	"void main() {"
		"color = vec4(intens, intens, intens, 1.0);"
	"}"
;

// x, y, intensity
GLfloat vertices[] = {
	10, 10, 1.0,
	10, 50, 0.0,
	50, 10, 1.0,
	50, 50, 0.0,
};
const int num_verts = sizeof(vertices) / (sizeof(GLfloat) * 3);

static void setup_vertices(GLuint* vao, GLuint* vbo) {
	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glBindVertexArray(*vao);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0); // in vec3 coord
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
		(void*)(2 * sizeof(GLfloat))); // in vec3 color
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int not_main() {
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
			.width = WIDTH,
			.height = HEIGHT,
		}
	);

	output_open(oc);

	float at = 0;
	float atincr = 2 * M_PI * 110.0 / oc->acc->sample_rate;
	float atincr2 = atincr / oc->acc->sample_rate;

	GfxContext* gc = gfx_create(WIDTH, HEIGHT);
	RenderContext* rc = render_create();

	TextContext* tc = text_create("sample.ttf", 10);
	RenderDrawable rd = text_render(tc, "Oliopolig", 60.0, 60.0);
	render_add(rc, &rd);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (output_is_open(oc)) {
		if (output_get_seconds(oc) >= 10.0)
			output_close(oc);

		if (output_get_encode_type(oc) == OUTPUT_TYPE_VIDEO) {
			glClearColor(0.0, 0.5, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			
			//glUseProgram(prog);
			//glBindVertexArray(vao);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
			render(rc, WIDTH, HEIGHT);

			gfx_render(gc, oc->vf);

			output_encode_video(oc);
		} else {
			int16_t *data = (int16_t *)oc->afd->data[0];
			for (int i = 0; i < oc->afd->nb_samples; i++) {
				int v = 10000 * sin(at);
				for (int j = 0; j < oc->acc->ch_layout.nb_channels; j++)
					*data++ = v;
				at += atincr;
				atincr += atincr2;
			}
			output_encode_audio(oc);
		}
	}

	//glDeleteVertexArrays(1, &vao);
	//glDeleteBuffers(1, &vbo);
	//glDeleteProgram(prog);

	text_destroy(tc);
	gfx_destroy(gc);
	output_destroy(oc);

	return 0;
}

int main(int argc, char** argv) {
	switch (ERROR_GET()) {
	case ERROR_NONE:
		break;
	default:
		LOG("Fatal error");
		return 1;
	}

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [SCRIPT]\n", argv[0]);
		return 1;
	}

	scripting_exec(argv[1]);
}

