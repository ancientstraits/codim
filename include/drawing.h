#ifndef DRAWING_H
#define DRAWING_H

#include <libavcodec/avcodec.h>

enum {
	COLOR_Y,
	COLOR_CB,
	COLOR_CR,
};
// Convert an RGB color to YCbCr.
uint8_t rgb_to_ycbcr(int type, uint8_t r, uint8_t g, uint8_t b);

typedef struct {
	uint8_t r, b, g;
} Color;
typedef struct {
	uint32_t x, y, width, height;
} Rect;
void draw_box(AVFrame* f, Rect r, Color fg, Color bg);

#endif // !DRAWING_H
