#include <epoxy/gl.h>
#include "error.h"
#include "glutil.h"

#define GASSERT(cond, ...) ASSERT(cond, ERROR_GL, __VA_ARGS__)

GLuint single_shader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(shader, 512, NULL, log);
		GASSERT(0, "%s shader error: %s",
			(type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment", log);
	}

	return shader;
}

GLuint shader_prog(const char* vert_source, const char* frag_source) {
	GLuint vert_shader = single_shader(GL_VERTEX_SHADER,   vert_source);
	GLuint frag_shader = single_shader(GL_FRAGMENT_SHADER, frag_source);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert_shader);
	glAttachShader(prog, frag_shader);
	glLinkProgram(prog);
	
	int success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		char log[512];
		glGetProgramInfoLog(prog, 512, NULL, log);
		GASSERT(false, "Program error: %s", log);
	}

	return prog;
}

