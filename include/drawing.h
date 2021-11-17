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

void draw_pixel(AVFrame *frame, int x, int y, int color);

/*
 * Blend color_a and color_b with a factor of `blend/255`.
 * The closer `blend` is to 255, the closer the result is to
 * color_b.
 */
int blend_colors(int color_a, int color_b, uint8_t blend);

void fill_frame(AVFrame *frame, int color);

typedef struct {
	uint32_t x, y, width, height;
} Rect;
// Draw a box. fg and bx are hexcodes.
void draw_box(AVFrame *f, Rect *r, int color);

#endif // !DRAWING_H
