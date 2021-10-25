#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "drawutils.h"
#include "drawing.h"
#include "text.h"

TextContext* text_context_init(const char *font_path, size_t font_size, int width, int height) {
	TextContext* tc = malloc(sizeof(TextContext));
	if (FT_Init_FreeType(&tc->lib)) {
		fprintf(stderr, "Could not initialize FreeType\n");
		return NULL;
	}
	if (FT_New_Face(tc->lib, font_path, 0, &tc->face)) {
		fprintf(stderr, "Failed to load font face from file %s\n", font_path);
		return NULL;
	}
	// TODO make resolution a parameter
	if (FT_Set_Char_Size(tc->face, 0, font_size * 64, width, height)) {
		fprintf(stderr, "Failed to set font size\n");
		return NULL;
	}
	FT_Matrix matrix;

	// const double angle = 10.0;
	// const double s_a = -1;
	// const double c_a = 1;
	// matrix.xx = (FT_Fixed)( 1 * 0x10000L);
	// matrix.xy = (FT_Fixed)( 0 * 0x10000L);
	// matrix.yx = (FT_Fixed)( 0 * 0x10000L);
	// matrix.yy = (FT_Fixed)(-2 * 0x10000L);
	matrix.xx = 0x10000L;
    matrix.xy = 0;
    matrix.yx = 0.12 * 0x10000L;
    matrix.yy = 0x10000L;
	FT_Set_Transform(tc->face, &matrix, NULL);
	return tc;
}

void text_context_delete(TextContext *tc) {
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
	free(tc);
}

// CREDIT: https://www.freetype.org/freetype2/docs/tutorial/example1.c
static void draw_bitmap(FT_Bitmap* bm, AVFrame* frame, int x, int y, int color) {
	int i, j, p, q;
	const int x_max = x + bm->width;
	const int y_max = y + bm->rows;
	for (i = x, p = 0; i < x_max; i++, p++) {
		for (j = y, q = 0; j < y_max; j++, q++) {
			// TODO add a way to detect width or height
			if ( i < 0      || j < 0       /*|| i >= WIDTH || j >= HEIGHT*/)
    			continue;
			if (bm->buffer[q * bm->width + p] > 3)
				draw_pixel(frame, i / 2, j, color);
		}
	}
}

int draw_text(TextContext* tc, AVFrame* frame, const char* str, int xpos, int ypos, int color) {
	FT_GlyphSlot slot = tc->face->glyph;
	int pen_x = xpos;
	int pen_y = ypos;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\n') {
			// Newline support
			pen_x = xpos;
			printf("PEN_Y: %d -> ", pen_y);
			pen_y += tc->face->height / 64;
			printf("%d\n", pen_y);
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
		draw_bitmap(&slot->bitmap, frame, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, color);
		printf("bearings: %d, %d", slot->bitmap_left, slot->bitmap_top);
		pen_x += slot->advance.x / 64;
		pen_y += slot->advance.y / 64;
		printf("\nPen: %d, %d\n", pen_x, pen_y);
	}
	return 0;
}
