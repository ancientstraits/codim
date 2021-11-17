#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "drawing.h"
#include "text.h"

int make_char(FontChar *fc, FT_Face face, char c) {
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, c), 0)) {
		fprintf(stderr, "Failed to load character in FreeType: '%c'\n", c);
		return 0;
	}
	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
		fprintf(stderr, "Failed to load character in FreeType: '%c'\n", c);
		return 0;
	}

	fc->advance = face->glyph->advance.x / 64;
	fc->bearing.x = face->glyph->bitmap_left;
	fc->bearing.y = face->glyph->bitmap_top;
	fc->size.x = face->glyph->bitmap.width;
	fc->size.y = face->glyph->bitmap.rows;
	fc->pitch = face->glyph->bitmap.pitch;
	fc->buf = face->glyph->bitmap.buffer;

	return 1;
}

void loop_font_chars(FontChar fc[256], FT_Face face) {
	for (unsigned char c = 0; c < 255; c++) {
		if (!make_char(&fc[c], face, c))
			fprintf(stderr, "Failed to create char '%c'\n", c);
	}
}

TextContext *text_context_init(const char *font_path, size_t font_size) {
	TextContext *tc = malloc(sizeof(TextContext));

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

	loop_font_chars(tc->chars, tc->face);

	return tc;
}

void text_context_delete(TextContext *tc) {
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
	free(tc);
}

static void draw_font_char(FontChar *fc, AVFrame *frame, int x, int y, int fg,
						   int bg) {

	int i, j, p, q;
	const int x_max = x + fc->size.x;
	const int y_max = y + fc->size.y;
	for (i = x, p = 0; i < x_max; i++, p++) {
		for (j = y, q = 0; j < y_max; j++, q++) {
			// TODO add a way to detect width or height
			if (i < 0 || j < 0 /*|| i >= WIDTH || j >= HEIGHT*/)
				continue;
			draw_pixel(frame, i, j,
					   blend_colors(bg, fg, fc->buf[q * fc->pitch + p]));
		}
	}
}

int draw_single_char(TextContext *tc, AVFrame *frame, char c, int xpos,
					 int ypos, int fg, int bg) {
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
	draw_font_char(&tc->chars[(int)c], frame,
				   tc->loc.x + tc->face->glyph->bitmap_left,
				   tc->loc.y - tc->face->glyph->bitmap_top, fg, bg);
	tc->loc.x += tc->face->glyph->advance.x / 64;
	tc->loc.y += tc->face->glyph->advance.y / 64;

	return 0;
}

int draw_text(TextContext *tc, AVFrame *frame, const char *str, int xpos,
			  int ypos, int fg, int bg) {
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
			fprintf(stderr, "Failed to load character in FreeType: '%c'\n",
					str[i]);
			return 1;
		}
		if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL)) {
			fprintf(stderr, "Failed to load character in FreeType: '%c'\n",
					str[i]);
			return 1;
		}
		// printf("FT Info: size: %dx%d", slot->bitmap.rows,
		// slot->bitmap.width);
		draw_font_char(&tc->chars[(int)str[i]], frame,
					   tc->loc.x + slot->bitmap_left,
					   tc->loc.y - slot->bitmap_top, fg, bg);
		tc->loc.x += slot->advance.x / 64;
		tc->loc.y += slot->advance.y / 64;
	}
	return 0;
}
