#include <ft2build.h>
#include FT_FREETYPE_H

// hello txt :D

#include <epoxy/gl.h>

#include "text.h"
#include "error.h"

#define TASSERT(cond, ...) ASSERT(cond, ERROR_TEXT, __VA_ARGS__);

#define FACE "C:/Windows/Fonts/Comic.ttf"

const char* vert_shader =
	"#version 330\n"

	"attribute vec4 coord;"
	"varying vec2 texcoord;"

	"void main(void) {"
		"gl_Position = vec4(coord.xy, 0, 1);"
		"texcoord = coord.zw;"
	"}"
;

const char* frag_shader =
	"#version 330\n"

	"varying vec2 texcoord;"
	"uniform sampler2D tex;"
	"uniform vec4 color;"

	"void main(void) {"
		"gl_FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) & color;"
	"}"
;

void text_something(void) {
	FT_Library lib;
	TASSERT(FT_Init_FreeType(&lib), "Could not initialize FreeType");
	FT_Face face;
	TASSERT(FT_New_Face(lib, FACE, 0, &face), "Could not open font");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

