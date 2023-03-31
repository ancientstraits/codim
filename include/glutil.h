#ifndef GLUTIL_H
#define GLUTIL_H

GLuint single_shader(GLenum type, const char* source);
GLuint shader_prog(const char* vert_source, const char* frag_source);

#endif // !GLUTIL_H

