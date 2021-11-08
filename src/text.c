#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "drawing.h"
#include "text.h"

TextContext* text_context_init(const char *font_path, size_t font_size) {
	TextContext* tc = malloc(sizeof(TextContext));
	if (FT_Init_FreeType(&tc->lib)) {
		fprintf(stderr, "Could not initialize FreeType\n");
		return NULL;
	}
	if (FT_New_Face(tc->lib, font_path, 0, &tc->face)) {
		fprintf(stderr, "Failed to load font face from the font path\n");
		return NULL;
	}

	if (FT_Set_Pixel_Sizes(tc->face, 0, font_size)) {
		fprintf(stderr, "Failed to set font size\n");
		return NULL;
	}

	tc->loc.x = 0;
	tc->loc.y = 0;

	return tc;
}

void text_context_delete(TextContext *tc) {
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
	free(tc);
}

static void draw_bitmap(FT_Bitmap* bm, AVFrame* frame, int x, int y, int fg, int bg) {
	int i, j, p, q;
	const int x_max = x + bm->width;
	const int y_max = y + bm->rows;
	for (i = x, p = 0; i < x_max; i++, p++) {
		for (j = y, q = 0; j < y_max; j++, q++) {
			// TODO add a way to detect width or height
			if ( i < 0      || j < 0       /*|| i >= WIDTH || j >= HEIGHT*/)
    			continue;
			draw_pixel(frame, i, j, blend_colors(bg, fg, bm->buffer[q * bm->pitch + p]));
		}
	}
}

int draw_single_char(TextContext* tc, AVFrame* frame, char c, int xpos, int ypos, int fg, int bg) {
	if (c == '\n') {
		tc->loc.x = xpos;
		tc->loc.y += tc->face->size->metrics.height / 64;
		return 0;
	}

	if (FT_Load_Glyph(tc->face, FT_Get_Char_Index(tc->face, c), 0)) {
		fprintf(stderr, "Failed to load character in FreeType: '%c'\n", c);
		return 1;
	}
	if (FT_Render_Glyph(tc->face->glyph, FT_RENDER_MODE_NORMAL)) {
		fprintf(stderr, "Failed to load character in FreeType: '%c'\n", c);
		return 1;
	}
	draw_bitmap(
		&tc->face->glyph->bitmap,
		frame,
		tc->loc.x + tc->face->glyph->bitmap_left,
		tc->loc.y - tc->face->glyph->bitmap_top,
		fg,
		bg
	);
	tc->loc.x += tc->face->glyph->advance.x / 64;
	tc->loc.y += tc->face->glyph->advance.y / 64;

	return 0;
}

int draw_text(TextContext* tc, AVFrame* frame, const char* str, int xpos, int ypos, int fg, int bg) {
	FT_GlyphSlot slot = tc->face->glyph;
	tc->loc.x = xpos;
	tc->loc.y = ypos + (tc->face->size->metrics.height / 64);
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\n') {
			// Newline support
			tc->loc.x = xpos;
			tc->loc.y += tc->face->size->metrics.height / 64;
			continue;
		}
		if (FT_Load_Glyph(tc->face, FT_Get_Char_Index(tc->face, str[i]), 0)) {
			fprintf(stderr, "Failed to load character in FreeType: '%c'\n", str[i]);
			return 1;
		}
		if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL)) {
			fprintf(stderr, "Failed to load character in FreeType: '%c'\n", str[i]);
			return 1;
		}
		// printf("FT Info: size: %dx%d", slot->bitmap.rows, slot->bitmap.width);
		draw_bitmap(&slot->bitmap, frame, tc->loc.x + slot->bitmap_left, tc->loc.y - slot->bitmap_top, fg, bg);
		tc->loc.x += slot->advance.x / 64;
		tc->loc.y += slot->advance.y / 64;
	}
	return 0;
}
