#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// #include <cglm/cglm.h>
#include <epoxy/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "error.h"
#include "text.h"

#define TASSERT(cond, ...) ASSERT(cond, ERROR_TEXT, __VA_ARGS__)

static void text_load(TextContext* tc, uint32_t c) {
	TASSERT(!FT_Load_Char(tc->face, c, FT_LOAD_RENDER), "Could not render %c character", c);	
}

void text_init(TextContext* tc, const char* font_path, uint32_t px_size) {
	// NOTE FreeType functions return 0 on success
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
		atlas_w += g->bitmap.width + 1; // the `1` is for padding
	}
	tc->atlas_w = atlas_w;
	tc->atlas_h = atlas_h;

	glGenTextures(1, &tc->tex);
	glBindTexture(GL_TEXTURE_2D, tc->tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED,
		atlas_w, atlas_h, 0,
		GL_RED, GL_UNSIGNED_BYTE, 0
	);

	// 2nd pass: construct atlas
	int x = 0;
	for (uint8_t c = 0; c < 128; c++) {
		text_load(tc, c);

		tc->info[c].adv_x  = g->advance.x / 64.0;
		tc->info[c].adv_y  = g->advance.y / 64.0;
		tc->info[c].width  = g->bitmap.width;
		tc->info[c].height = g->bitmap.rows;
		tc->info[c].left   = g->bitmap_left;
		tc->info[c].top    = g->bitmap_top;
		tc->info[c].off_x  = x;

		// Do not add to texture if is space
		// (if you do, OpenGL will give warning)
		if (c != ' ') {
			glTexSubImage2D(
				GL_TEXTURE_2D, 0,
				x, 0, g->bitmap.width, g->bitmap.rows,
				GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer
			);
		}
		x += g->bitmap.width + 1; // the `+ 1` is for padding
	}
	tc->space_w = tc->info[' '].adv_x;
	tc->line_h  = px_size;
}

// TODO this is only a sample!
// This code should be replaced with code that loads texture atlas into OpenGL!
void text_render_px(TextContext* tc, AVFrame* f) {
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

void text_coord_create_vertices(GLuint* vao, GLuint* vbo, TextCoord* coords, size_t coord_len) {
	// glClearColor(0.0, 0.0, 1.0, 1.0);
	// glClear(GL_COLOR_BUFFER_BIT);
	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glBindVertexArray(*vao);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, coord_len * sizeof(TextCoord), coords, GL_DYNAMIC_DRAW);

	// glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TextCoord), (void*)0); // in vec4 coord
	// glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextCoord), (void*)0); // in vec2 pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextCoord), (void*)(2*sizeof(GLfloat))); // in vec2 tex_coord
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

size_t text_coord_build(TextCoord* out, TextContext* tc, const char* s, float tx, float ty, float* ox, float* oy) {
	int idx = 0;
	float x = tx, y = ty;
	float aw = tc->atlas_w, ah = tc->atlas_h;

	for (int i = 0; s[i]; i++) {
		int c = s[i];	
		if (c == '\n') {
			x = tx;
			y -= tc->line_h;
			continue;
		}
		if (c == ' ') {
			x += tc->space_w;
		}

		float cx = x + tc->info[c].left;
		float cy = y + tc->info[c].top;
		float w = tc->info[c].width;
		float h = tc->info[c].height;
		float off = tc->info[c].off_x;

		// Can skip drawing if width or height is 0
		if (w == 0 || h == 0)
			continue;

		// coord = (Coord){x, y, s, t};
		// Use normalized coords, since computing them first will be
		// faster than trying to do them each frame with shaders.
		// 1  3
		// | /|
		// |/ |
		// 2  4
		out[idx++] = (TextCoord){cx,     cy,     off  /  aw, 0.0}; // Upper Left
		out[idx++] = (TextCoord){cx,     cy - h, off  /  aw, h/ah}; // Lower Left
		out[idx++] = (TextCoord){cx + w, cy,     (off+w)/aw, 0.0}; // Upper Right
	
		out[idx++] = (TextCoord){cx,     cy - h, off  /  aw, h/ah}; // Lower Left
		out[idx++] = (TextCoord){cx + w, cy,     (off+w)/aw, 0.0}; // Upper Right
		out[idx++] = (TextCoord){cx + w, cy - h, (off+w)/aw, h/ah}; // Lower Right

		//coords[idx++] = (Coord){-0.5, 0.5, 0.0, 1.0}; // Upper Left
		//coords[idx++] = (Coord){-0.5, -0.5, 0.0, 0.0}; // Lower Left
		//coords[idx++] = (Coord){0.5, 0.5, 1.0, 1.0}; // Upper Right
		//coords[idx++] = (Coord){0.5, -0.5, 1.0, 0.0}; // Lower Right

		x += tc->info[c].adv_x;
		y += tc->info[c].adv_y;
	}

	if (ox) *ox = x;
	if (oy) *oy = y;

	return idx;
}

// Assume pixel coords for `x` and `y`
RenderDrawable text_render(TextContext* tc, const char* s, float tx, float ty) {
	const size_t coord_len = 6 * strlen(s);

	TextCoord* coords = malloc(coord_len * sizeof(TextCoord));

	size_t n_coords = text_coord_build(coords, tc, s, tx, ty, NULL, NULL);

	GLuint vao, vbo;
	text_coord_create_vertices(&vao, &vbo, coords, n_coords);

	free(coords);

	RenderDrawable ret = {
		.vao = vao,
		// .vbo = vbo,
		.tex = tc->tex,
		.n_verts = n_coords,
		.draw_type = RENDER_DRAW_XYST,
		.draw_flags = RENDER_DRAW_FLAG_R_TEXTURE,
	};
	mat4x4_identity(ret.model);
	return ret;

	// rd->vao = vao;
	// rd->vbo = vbo;
	// rd->tex = tc->tex;
	// rd->n_verts = idx;
	// rd->texdim.x = tc->atlas_w;
	// rd->texdim.y = tc->atlas_h;
}

void text_deinit(TextContext* tc) {
	glDeleteTextures(1, &tc->tex);
	FT_Done_Face(tc->face);
	FT_Done_FreeType(tc->lib);
}

