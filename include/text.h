#ifndef CODIM_TEXT_H
#define CODIM_TEXT_H

// `text.h` offers various utilities for text in Codim.

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <libavcodec/avcodec.h>

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
	GLuint vao, vbo, prog, tex;
	uint32_t num_verts, atlas_w, atlas_h;

	TextCharInfo info[128];
} TextContext;


TextContext* text_create(const char* font_path, uint32_t px_size);
//void text_load(TextContext* tc, uint32_t c);
void text_render(TextContext* tc, const char* s, float x, float y, float sw, float sh);
void text_draw(TextContext* tc, float width, float height);
void text_destroy(TextContext* tc);


#endif // !CODIM_TEXT_H

