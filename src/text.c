#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "error.h"
#include "text.h"

#define TASSERT(cond, ...) ASSERT(cond, ERROR_TEXT, __VA_ARGS__)


static void text_load(TextContext* tc, uint32_t c) {
	TASSERT(!FT_Load_Char(tc->face, c, FT_LOAD_RENDER), "Could not render %c character", c);	
}

TextContext* text_create(const char* font_path, uint32_t px_size) {
	TextContext* tc = calloc(1, sizeof *tc);

	// XXX FreeType functions return 0 on success
	// Make sure all ASSERT conds for FreeType functions start with `!`
	TASSERT(!FT_Init_FreeType(&tc->lib), "Failed to initialize FreeType");
	TASSERT(!FT_New_Face(tc->lib, font_path, 0, &tc->face),
		"Could not open font file %s", font_path);

	TASSERT(!FT_Set_Pixel_Sizes(tc->face, 0, px_size), "Failed to set font size");


	uint32_t atlas_w = 0, atlas_h = 0;
	FT_GlyphSlot g = tc->face->glyph;

	// 1st pass: determine dimensions
	for (uint8_t c = 0; c < 128; c++) {
		text_load(tc, c);
		if (atlas_h < g->bitmap.rows)
			atlas_h = g->bitmap.rows;
		atlas_h += g->bitmap.width;
	}

	return tc;
}

// TODO this is only a sample!
// This code should be replaced with code that loads texture atlas into OpenGL!
void text_render(TextContext* tc, AVFrame* f) {
	FT_GlyphSlot g = tc->face->glyph;

	char* s = "Olipolig";

	uint32_t ox = 10, oy = 30;
	for (int i = 0; s[i]; i++) {
		text_load(tc, s[i]);

		for (uint32_t y = 0; y < g->bitmap.rows; y++) {
			for (uint32_t x = 0; x < g->bitmap.width; x++) {
				uint32_t fx = x + ox + g->bitmap_left;
				uint32_t fy = y + oy - g->bitmap_top;
				f->data[0][fy * f->linesize[0] + fx] = g->bitmap.buffer[y * g->bitmap.width + x];
			}
		}
		ox += g->advance.x / 64;
		oy += g->advance.y / 64;
	}
}

void text_destroy(TextContext* tc) {
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
	free(tc);
}

