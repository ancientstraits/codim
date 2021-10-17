#ifndef TEXT_H
#define TEXT_H

#include <libavcodec/avcodec.h>
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	FT_Library lib;
	FT_Face face;
} TextContext;

TextContext* text_context_init(const char* font_path, size_t font_size, int width, int height);
void text_context_delete(TextContext* tc);

int draw_text(TextContext* tc, AVFrame* frame, const char* str, int x, int y, int color);

#endif // !TEXT_H
