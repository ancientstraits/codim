#ifndef RENDER_H
#define RENDER_H

#include <cglm/struct.h>
#include <epoxy/gl.h>

typedef enum {
	RENDER_DRAW_XYST,  // Requires texture
	// RENDER_DRAW_XYZST, // Requires texture
	RENDER_DRAW_XY,
	// RENDER_DRAW_XYZ,
	RENDER_DRAW_XYRGB,
	// RENDER_DRAW_XYZRGB,
	// RENDER_DRAW_XYRGBA,
	// RENDER_DRAW_XYZRGBA,
} RenderDrawType;

typedef enum {
	RENDER_DRAW_FLAG_R_TEXTURE    = 1,
	RENDER_DRAW_FLAG_RGB_TEXTURE  = (1 << 1),
	RENDER_DRAW_FLAG_RGBA_TEXTURE = (1 << 2),
} RenderDrawFlags;

typedef struct {
	// vec2s texdim;
	RenderDrawType draw_type;
	RenderDrawFlags draw_flags;

	// set `tex` to 0 if not using
	GLuint tex, vao;
	GLuint prog;
	GLuint n_verts;
	// To transform the object
	mat4s model;
} RenderDrawable;

typedef struct {
	RenderDrawable** drawables;
	// size_t len, capacity;
	GLuint progs[5];
} RenderContext;

// RenderContext* render_create();
void render_init(RenderContext* rc);
void render_add(RenderContext* rc, RenderDrawable* rd);
void render_del(RenderContext* rc, RenderDrawable* rd);
void render(RenderContext* rc, int width, int height);
void render_deinit(RenderContext* rc);

#endif // !RENDER_H

