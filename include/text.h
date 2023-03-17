#ifndef CODIM_TEXT_H
#define CODIM_TEXT_H

// `text.h` offers various utilities for text in Codim.

#include <ft2build.h>
#include FT_FREETYPE_H
#include <libavcodec/avcodec.h>

typedef struct TextContext {
	FT_Library lib;
	FT_Face face;
} TextContext;


TextContext* text_create(const char* font_path, uint32_t px_size);
void text_render(TextContext* tc, AVFrame* f);
void text_destroy(TextContext* tc);


#endif // !CODIM_TEXT_H

