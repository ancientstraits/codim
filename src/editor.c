#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "error.h"
#define EASSERT(cond, ...) ASSERT(cond, ERROR_EDITOR, __VA_ARGS__);

void editor_init(EditorContext* ec, TextContext* tc, int x, int y, int w, int h) {
    ec->capacity = 4096; // Estimate; replace this with calculations later
    ec->tc = tc;
    ec->buffer = malloc(ec->capacity);
    ec->coords = malloc(ec->capacity * sizeof(TextCoord));
    ec->gap_start = ec->buffer;
    ec->gap_end = ec->buffer + ec->capacity;
    ec->x = x;
    ec->y = y;

    text_coord_create_vertices(&ec->rd.vao, &ec->vbo, NULL, ec->capacity); // Check if `&(TextCoord){}` can be replaced with `NULL`

    ec->rd.draw_type  = RENDER_DRAW_XYST;
    ec->rd.draw_flags = RENDER_DRAW_FLAG_R_TEXTURE;
    ec->rd.n_verts    = 0;
    ec->rd.tex        = tc->tex;
}

void editor_deinit(EditorContext* ec) {
    free(ec->buffer);
}

void editor_update(EditorContext* ec) {
    ec->gap_start[0] = 0; // null terminator trick

    float ox, oy;
    size_t n_coords = text_coord_build(ec->coords, ec->tc, ec->buffer, ec->x, ec->y, &ox, &oy);
    n_coords += text_coord_build(&ec->coords[n_coords], ec->tc, ec->gap_end, ox, oy, &ox, &oy);

    glBindBuffer(GL_ARRAY_BUFFER, ec->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TextCoord) * n_coords, ec->coords);

    ec->rd.n_verts = n_coords;
}

void editor_insert(EditorContext* ec, const char* text) {
    strcpy(ec->gap_start, text);
    ec->gap_start += strlen(text);

    editor_update(ec);
}

void editor_delete(EditorContext* ec, EditorSeekType seek_type, size_t start, size_t end) {
    if (seek_type == EDITOR_SEEK_CUR) {
        ec->gap_start += start;
        ec->gap_end += end;
    } else if (seek_type == EDITOR_SEEK_END) {

    } else {
    }
}

void editor_move_cursor(EditorContext* ec, EditorSeekType seek_type, int offset) {
    int rel_offset;
    if (seek_type == EDITOR_SEEK_CUR) {
        rel_offset = offset;
    } else if (seek_type == EDITOR_SEEK_END) {
        rel_offset = ((ec->buffer + ec->capacity) - ec->gap_end) - offset;
    } else {
        rel_offset = offset - (ec->gap_start - ec->buffer);
    }

    if (rel_offset == 0) return;
    else if (rel_offset < 0) {
        int o = -rel_offset; // o is now positive

        ec->gap_end -= o;
        char* new_gap_start = ec->gap_start - o;
        memcpy(ec->gap_end, new_gap_start, o);
        ec->gap_start = new_gap_start;
    } else {
        memcpy(ec->gap_start, ec->gap_end, rel_offset);
        ec->gap_start += rel_offset;
        ec->gap_end += rel_offset;
    }

    if (ec->gap_start < ec->buffer)
        ec->gap_start = ec->buffer;
    if (ec->gap_end > ec->buffer + ec->capacity)
        ec->gap_end = ec->buffer + ec->capacity;
}

size_t editor_get_cursor_pos(EditorContext *ec) {
    return ec->gap_start - ec->buffer;
}

void editor_clear(EditorContext* ec) {
    ec->gap_start = ec->buffer;
    ec->gap_end = ec->buffer + ec->capacity;

    editor_update(ec);
}