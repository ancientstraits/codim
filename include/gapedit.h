#ifndef GAPEDIT_H
#define GAPEDIT_H

typedef struct {
	size_t gap_len;
	char* buf;
	size_t gap_start, gap_end;
} GadEdit;

#endif // !GAPEDIT_H

