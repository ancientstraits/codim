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
	GLuint tex;

	TextCharInfo info[128];
} TextContext;


TextContext* text_create(const char* font_path, uint32_t px_size);
//void text_load(TextContext* tc, uint32_t c);
RenderDrawable text_render(TextContext* tc,
	const char* s, float x, float y);
void text_destroy(TextContext* tc);


#endif // !CODIM_TEXT_H

