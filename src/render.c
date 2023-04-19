#include <stdio.h>
#include <stdint.h>
#include "render.h"

RenderContext* render_create() {
	RenderContext* rc = malloc(sizeof *rc);
	rc->num_objects = 0;
	rc->capacity = 1;
	rc->objects = malloc(sizeof(RenderObject) * rc->capacity);

	return rc;
}

void render_add(RenderContext* rc, RenderDrawable* rd) {
	size_t idx = rc->num_objects++;
	if (rc->num_objects > rc->capacity) {
		rc->capacity *= 2;
		rc->objects = realloc(rc->objects, rc->capacity);
	}
	RenderObject* obj = &rc->objects[idx];
	obj->use_txt = (rd->txt.ptr) ? 1 : 0;

	glGenVertexArrays(1, &obj->vao);
	glGenBuffers(1, &obj->vbo);
	glBindVertexArray(obj->vao);
	glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
	glBufferData(GL_ARRAY_BUFFER, obj->n_vertices * sizeof(RenderVertex),
		obj->vertices, GL_DYNAMIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(RenderVertex), (void*)0); // vec3 pos
	glEnableVertexAttribArray(0);

	if (obj->use_txt) {
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
			sizeof(RenderVertex), (void*)(sizeof(vec3))); // vec3 texpos
		glEnableVertexAttribArray(1);
	}
}

