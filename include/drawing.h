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

void draw_pixel(AVFrame* frame, int x, int y, int color);

typedef struct {
	uint32_t x, y, width, height;
} Rect;
// Draw a box. fg and bx are hexcodes.
void draw_box(AVFrame* f, Rect r, int fg, int bg);

#endif // !DRAWING_H
