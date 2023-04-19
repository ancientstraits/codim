#ifndef GLUTIL_H
#define GLUTIL_H

#define GL() do { \
	GLenum x = glGetError(); \
	ASSERT(x == GL_NO_ERROR, ERROR_GFX, "Error %u", x); \
} while(0)

GLuint single_shader(GLenum type, const char* source);
GLuint shader_prog(const char* vert_source, const char* frag_source);

#endif // !GLUTIL_H

