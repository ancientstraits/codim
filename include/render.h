#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <epoxy/gl.h>
#include <cglm/cglm.h>

typedef struct RenderObject {
	int use_txt;
	GLuint vao, vbo, txt, shader;
} RenderObject;

typedef struct RenderVertex {
	vec3 pos, texpos;
} RenderVertex;

typedef struct RenderTexture {
	uint8_t* ptr;
	vec2 dim;
} RenderTexture;

typedef struct RenderDrawable {
	vec3 pos;
	size_t n_vertices;
	// if txt.ptr == NULL, doesn't use texture
	RenderTexture txt;
	RenderVertex* vertices;
} RenderDrawable;

typedef struct RenderContext {
	size_t num_objects, capacity;
	RenderObject* objects;
} RenderContext;

RenderContext* render_create();
void render_add(RenderContext* rc, RenderDrawable* rd);
void render(RenderContext* rc);
void render_destroy(RenderContext* rc);

#endif // !RENDER_H

