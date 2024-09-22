#ifndef GAPEDIT_H
#define GAPEDIT_H

#include "render.h"
#include "text.h"

typedef struct EditorContext {
	char* buffer, *gap_start, *gap_end;
	int x, y;
	// vec2s* positions;
	TextContext* tc;
	TextCoord* coords;
	size_t capacity;
	GLuint vbo;
	
	RenderDrawable rd;
} EditorContext;

typedef enum EditorSeekType {
	EDITOR_SEEK_SET,
	EDITOR_SEEK_CUR,
	EDITOR_SEEK_END,
} EditorSeekType;

void editor_init(EditorContext* ec, TextContext* tc, int x, int y, int w, int h);
void editor_insert(EditorContext* ec, const char* text);
void editor_insert_char(EditorContext* ec, char c);
void editor_delete(EditorContext* ec, EditorSeekType seek_type, size_t start, size_t end);
void editor_move_cursor(EditorContext* ec, EditorSeekType seek_type, int offset);
size_t editor_get_cursor_pos(EditorContext* ec);
void editor_clear(EditorContext* ec);
void editor_deinit(EditorContext* ec);

#endif // !GAPEDIT_H

