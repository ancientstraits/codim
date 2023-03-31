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

	int txt; // OpenGL texture ID
	TextCharInfo info[128];
} TextContext;


TextContext* text_create(const char* font_path, uint32_t px_size);
uint32_t text_max_rows(TextContext* tc);
void text_render(TextContext* tc, AVFrame* f);
void text_destroy(TextContext* tc);


#endif // !CODIM_TEXT_H

