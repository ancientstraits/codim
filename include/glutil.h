#ifndef GLUTIL_H
#define GLUTIL_H

#include <stdint.h>
#include <epoxy/gl.h>

#include "render.h"

#define GL() do { \
	GLenum x = glGetError(); \
	ASSERT(x == GL_NO_ERROR, ERROR_GFX, "Error %u", x); \
} while(0)

void setup_vertices(GLfloat* vertices, size_t n_verts, GLuint* vao, GLuint* vbo);
GLuint single_shader(GLenum type, const char* source);
GLuint shader_prog_from_ids(GLuint vert_shader, GLuint frag_shader);
GLuint shader_prog(const char* vert_source, const char* frag_source);

#endif // !GLUTIL_H

