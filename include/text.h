#ifndef CODIM_TEXT_H
#define CODIM_TEXT_H

// `text.h` offers various utilities for text in Codim.

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <libavcodec/avcodec.h>
#include "render.h"

typedef struct TextCharInfo {
	float
		adv_x, adv_y,
		width, height,
		left, top,
		off_x;
} TextCharInfo;

typedef struct TextContext {
	FT_Library lib;
	FT_Face face;
	uint32_t atlas_w, atlas_h;
	float space_w, line_h;
	GLuint tex;

	TextCharInfo info[128];
} TextContext;

typedef struct TextCoord {
	GLfloat x, y, s, t;
} TextCoord;


void text_init(TextContext* tc, const char* font_path, uint32_t px_size);
//void text_load(TextContext* tc, uint32_t c);
RenderDrawable text_render(TextContext* tc, const char* s, float x, float y);
void text_deinit(TextContext* tc);

void text_render_px(TextContext* tc, AVFrame* f);

void text_coord_create_vertices(GLuint* vao, GLuint* vbo, TextCoord* coords, size_t coord_len);
size_t text_coord_build(TextCoord* out, TextContext* tc, const char* s, float tx, float ty, float* ox, float* oy);


#endif // !CODIM_TEXT_H

