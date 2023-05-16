#include <stdio.h>
#include "error.h"
#define RASSERT(cond, ...) ASSERT(cond, ERROR_RENDER, __VA_ARGS__)
#include "glutil.h"
#include "render.h"
#include "stb_ds.h"

#define UNIFORM(prog, name, type, ...) \
	glUniform##type(glGetUniformLocation(prog, name), __VA_ARGS__)

static const char* vert_code =
	"#version 410 core\n"

	"uniform vec2 windim;\n"
	"uniform vec2 texdim;\n"
	//"uniform mat4 model, view, projection;"

	"layout (location = 0) in vec4 coord;\n"
	"out vec2 texcoord;\n"

	"vec2 ndc(vec2 coord, vec2 dim) {\n"
		"return (2.0 * (coord + 0.5) / dim) - 1.0;\n"
	"}\n"

	"void main() {\n"
		"texcoord = coord.zw / texdim;\n"
		"vec2 norm = ndc(coord.xy, windim);\n"
		"gl_Position = vec4(norm.x, norm.y * -1, 0.0, 1.0);\n"
	"}\n"
;
static const char* frag_code =
	"#version 410 core\n"
	"uniform sampler2D tex;"
	"in vec2 texcoord;"

	"void main() {"
		"gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(tex, texcoord).r);"
	"}"
;

RenderContext* render_create(void) {
	RenderContext* rc = malloc(sizeof* rc);

	rc->len = 0;
	rc->capacity = 1;
	rc->drawables = NULL; // stb_ds array
	rc->prog = shader_prog(vert_code, frag_code);

	return rc;
}

void render_add(RenderContext* rc, RenderDrawable rd) {
	rc->len++;
	arrpush(rc->drawables, rd);
}

void render(RenderContext* rc, int width, int height) {
	for (int i = 0; i < rc->len; i++) {
		RenderDrawable* rd = &(rc->drawables[i]);
		glBindTexture(GL_TEXTURE_2D, rd->tex);
		glUseProgram(rc->prog);
		glBindVertexArray(rd->vao);

		UNIFORM(rc->prog, "windim", 2f, width, height);
		UNIFORM(rc->prog, "texdim", 2f, rd->texdim[0], rd->texdim[1]);

		glDrawArrays(GL_TRIANGLES, 0, rd->n_verts);
	}
}

void render_destroy(RenderContext* rc) {
	arrfree(rc->drawables);
	glDeleteProgram(rc->prog);
	free(rc->drawables);
	free(rc);
}

