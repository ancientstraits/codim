#include <epoxy/gl.h>
#include "error.h"
#include "glutil.h"

#define GASSERT(cond, ...) ASSERT(cond, ERROR_GL, __VA_ARGS__)

void setup_vertices(GLfloat* vertices, size_t n_verts, GLuint* vao, GLuint* vbo) {
	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glBindVertexArray(*vao);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * n_verts, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0); // in vec3 coord
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
		(void*)(2 * sizeof(GLfloat))); // in vec3 color
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


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
	return shader_prog_from_ids(vert_shader, frag_shader);
}

GLuint shader_prog_from_ids(GLuint vert_shader, GLuint frag_shader) {
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
