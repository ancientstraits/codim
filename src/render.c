#include <stdio.h>
#include "error.h"
#define RASSERT(cond, ...) ASSERT(cond, ERROR_RENDER, __VA_ARGS__)
#include "glutil.h"
#include "render.h"
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define UNIFORM(prog, name, type, ...) \
	glUniform##type(glGetUniformLocation(prog, name), __VA_ARGS__)

#define VERT_CODE_UTIL_FNS \
	"vec2 ndc(vec2 coord, vec2 dim) {\n" \
		"return (2.0 * (coord + 0.5) / dim) - 1.0;\n" \
	"}\n"

// static const char* vert_code =
// 	"#version 410 core\n"

// 	"uniform sampler2D tex;"

// 	"uniform vec2 windim;\n"
// 	// "uniform vec2 texdim;\n"
// 	//"uniform mat4 model, view, projection;"

// 	"layout (location = 0) in vec4 coord;\n"
// 	"out vec2 texcoord;\n"

// 	"vec2 ndc(vec2 coord, vec2 dim) {\n"
// 		"return (2.0 * (coord + 0.5) / dim) - 1.0;\n"
// 	"}\n"

// 	"void main() {\n"
// 		// "texcoord = coord.zw / texdim;\n"
// 		"texcoord = coord.zw;\n"
// 		"vec2 norm = ndc(coord.xy, windim);\n"
// 		"gl_Position = vec4(norm.x, norm.y * -1, 0.0, 1.0);\n"
// 	"}\n"
// ;

// fragment shaders for XYST and XYZST
static const char* frag_code_rgb_tex =
	"#version 410 core\n"
	"uniform sampler2D tex;"
	"in vec2 texcoord;"

	"void main() {"
		"gl_FragColor = vec4(texture2D(tex, texcoord).rgb, 1.0);"
	"}"
;
static const char* frag_code_rgba_tex =
	"#version 410 core\n"
	"uniform sampler2D tex;"
	"in vec2 texcoord;"

	"void main() {"
		"gl_FragColor = texture2D(tex, texcoord);"
	"}"
;
static const char* frag_code_r_tex =
	"#version 410 core\n"
	"uniform sampler2D tex;"
	"in vec2 texcoord;"

	"void main() {"
		"gl_FragColor = vec4(1.0, 1.0, 1.0, texture2D(tex, texcoord).r);"
	"}"
;
// fragment shader for anything else
static const char* frag_code_no_tex =
	"#version 410 core\n"

	"in vec4 color;"

	"void main() {"
		"gl_FragColor = color;"
	"}"
;

static const char* vert_codes[] = {
	[RENDER_DRAW_XYST] =
		"#version 410 core\n"

		"uniform vec2 windim;"
		"uniform mat4 model;"

		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 tcoord;"
		"out vec2 texcoord;"


		VERT_CODE_UTIL_FNS

		"void main() {"
			"texcoord = tcoord;"

			"vec4 c = model * vec4(pos, 0.0, 1.0);"
			"vec2 norm = ndc(c.xy, windim);"
			// "vec2 norm = ndc(pos, windim);"
			"gl_Position = vec4(norm.x, -norm.y, 0.0, 1.0);"
		"}"
	,
	[RENDER_DRAW_XY] =
		"#version 410 core\n"

		"uniform vec2 windim;"
		"uniform mat4 model;"

		"layout (location = 0) in vec2 coord;"

		"out vec4 color;"

		VERT_CODE_UTIL_FNS

		"void main() {"
			"vec4 c = model * vec4(coord, 0.0, 1.0);"
			"vec2 norm = ndc(c.xy, windim);"
			"color = vec4(1.0, 1.0, 1.0, 1.0);"
			"gl_Position = vec4(norm.x, -norm.y, 0.0, 1.0);"
		"}"
	,
	[RENDER_DRAW_XYRGB] =
		"#version 410 core\n"

		"uniform vec2 windim;"
		"uniform mat4 model;"

		"layout (location = 0) in vec2 coord;"
		"layout (location = 1) in vec3 col;"

		"out vec4 color;"

		VERT_CODE_UTIL_FNS

		"void main() {"
			"vec4 c = model * vec4(coord, 0.0, 1.0);"
			"vec2 norm = ndc(c.xy, windim);"
			"color = vec4(col, 1.0);"
			"gl_Position = vec4(norm.x, -norm.y, 0.0, 1.0);"
		"}"
	,
};

GLuint progs[5] = {0};

// RenderContext* render_create(void) {
// 	RenderContext* rc = malloc(sizeof* rc);

// 	rc->len = 0;
// 	rc->capacity = 1;
// 	rc->drawables = NULL; // stb_ds array
// 	rc->prog = shader_prog(vert_code, frag_code);

// 	return rc;
// }
void render_init(RenderContext* rc) {
	rc->drawables = NULL; // stb_ds array
	// rc->prog = shader_prog(vert_code, frag_code);
	GLuint vert_xyst  = single_shader(GL_VERTEX_SHADER, vert_codes[RENDER_DRAW_XYST]);
	GLuint vert_xy    = single_shader(GL_VERTEX_SHADER, vert_codes[RENDER_DRAW_XY]);
	GLuint vert_xyrgb = single_shader(GL_VERTEX_SHADER, vert_codes[RENDER_DRAW_XYRGB]);

	GLuint frag_r_tex    = single_shader(GL_FRAGMENT_SHADER, frag_code_r_tex);
	GLuint frag_rgb_tex  = single_shader(GL_FRAGMENT_SHADER, frag_code_rgb_tex);
	GLuint frag_rgba_tex = single_shader(GL_FRAGMENT_SHADER, frag_code_rgba_tex);
	GLuint frag_no_tex   = single_shader(GL_FRAGMENT_SHADER, frag_code_no_tex);

	rc->progs[0] = shader_prog_from_ids(vert_xyst,  frag_r_tex);
	rc->progs[1] = shader_prog_from_ids(vert_xyst,  frag_rgb_tex);
	rc->progs[2] = shader_prog_from_ids(vert_xyst,  frag_rgba_tex);
	rc->progs[3] = shader_prog_from_ids(vert_xy,    frag_no_tex);
	rc->progs[4] = shader_prog_from_ids(vert_xyrgb, frag_no_tex);
}

static int determine_prog_number(RenderDrawType type, RenderDrawFlags flags) {
	if (type == RENDER_DRAW_XY) {
		return 3;
	} else if (type == RENDER_DRAW_XYRGB) {
		return 4;
	} else {
		if (flags & RENDER_DRAW_FLAG_R_TEXTURE)
			return 0;
		else if (flags & RENDER_DRAW_FLAG_RGB_TEXTURE)
			return 1;
		else
			return 2;
	}
}

typedef struct {
	int x, y;
} What;
typedef What* W;

void render_add(RenderContext* rc, RenderDrawable* rd) {
	rd->prog = rc->progs[determine_prog_number(rd->draw_type, rd->draw_flags)];
	arrpush(rc->drawables, rd);
}

void render_del(RenderContext* rc, RenderDrawable* rd) {
	for (int i = 0; i < arrlen(rc->drawables); i++) {
		if (rd == rc->drawables[i])
			arrdel(rc->drawables, i);
	}
}

void render(RenderContext* rc, int width, int height) {
	for (int i = 0; i < arrlen(rc->drawables); i++) {
		RenderDrawable* rd = rc->drawables[i];
		// printf("(%d, %d)\n", rd->texdim[0], rd->texdim[1]);
		glBindTexture(GL_TEXTURE_2D, rd->tex);
		glUseProgram(rd->prog);
		glBindVertexArray(rd->vao);

		printf("tex=%d, prog=%d, vao=%d\n", rd->tex, rd->prog, rd->vao);


		UNIFORM(rd->prog, "windim", 2f, width, height);
		UNIFORM(rd->prog, "model", Matrix4fv, 1, GL_FALSE, (float*)rd->model);

		glDrawArrays(GL_TRIANGLES, 0, rd->n_verts);
	}
}

void render_deinit(RenderContext* rc) {
	arrfree(rc->drawables);
	for (int i = 0; i < 5; i++)
		glDeleteProgram(rc->progs[i]);
	free(rc->drawables);
	// free(rc);
}

