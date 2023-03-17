#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "error.h"
#include "text.h"

#define TASSERT(cond, ...) ASSERT(cond, ERROR_TEXT, __VA_ARGS__)


TextContext* text_create(const char* font_path, uint32_t px_size) {
	TextContext* tc = calloc(1, sizeof *tc);

	// XXX FreeType functions return 0 on success
	// Make sure all ASSERT conds for FreeType functions start with `!`
	TASSERT(!FT_Init_FreeType(&tc->lib), "Failed to initialize FreeType");
	TASSERT(!FT_New_Face(tc->lib, font_path, 0, &tc->face),
		"Could not open font file %s", font_path);

	TASSERT(!FT_Set_Pixel_Sizes(tc->face, 0, px_size), "Failed to set font size");

	return tc;
}

// TODO this is only a sample!
// This code should be replaced with code that loads texture atlas into OpenGL!
void text_render(TextContext* tc, AVFrame* f) {
	TASSERT(!FT_Load_Char(tc->face, 'p', FT_LOAD_RENDER), "Could not render 'p' character");
	FT_Bitmap* b = &tc->face->glyph->bitmap;

	for (int y = 0; y < b->rows; y++) {
		for (int x = 0; x < b->width; x++) {
			f->data[0][y * f->linesize[0] + x] = b->buffer[y * b->width + x];
		}
	}
}

void text_destroy(TextContext* tc) {
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
	free(tc);
}

