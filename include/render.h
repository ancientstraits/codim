#ifndef RENDER_H
#define RENDER_H

#include <cglm/struct.h>
#include <epoxy/gl.h>

typedef struct {
	vec2s texdim;
	GLuint tex, vao, vbo;
	GLuint n_verts;
} RenderDrawable;

typedef struct {
	RenderDrawable* drawables;
	size_t len, capacity;
	GLuint prog;
} RenderContext;

// RenderContext* render_create();
void render_init(RenderContext* rc);
void render_add(RenderContext* rc, RenderDrawable rd);
void render(RenderContext* rc, int width, int height);
void render_deinit(RenderContext* rc);

#endif // !RENDER_H

