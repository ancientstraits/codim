#include <stdint.h>
#include <stdio.h>

#include "drawing.h"

uint8_t rgb_to_ycbcr(int type, uint8_t r, uint8_t g, uint8_t b) {
	switch (type) {
	case COLOR_Y:
		return 16 + (((r << 6) + (r << 1) + (g << 7) + g
		+ (b << 4) + (b << 3) + b) >> 8);
	case COLOR_CB:
		return 128 + (((-((r << 5) + (r << 2) + (r << 1)) -
				((g << 6) + (g << 3)) + (g << 1)) + 
				(b << 7) - (b << 4)) >> 8);
	case COLOR_CR:
		return 128 + (((r << 7) - (r << 4) -
				((g << 6) + (g << 5) - (g << 1)) -
				((b << 4) + (b << 1))) >> 8);

	default:
		return 0;
	}
}

uint8_t hexcode_to_ycbcr(int type, int hexcode) {
	return rgb_to_ycbcr(type, (hexcode & 0xff0000) >> 16, (hexcode & 0x00ff00) >> 8, hexcode & 0x0000ff);
}

/*
 * Blend color_a and color_b with a factor of `blend/255`.
 * The closer `blend` is to 255, the closer the result is to
 * color_b.
 */
int blend_colors(int color_a, int color_b, uint8_t blend) {
	const uint8_t a[3] = {(color_a & 0xff0000) >> 16, (color_a & 0x00ff00) >> 8, color_a & 0x0000ff};
	const uint8_t b[3] = {(color_b & 0xff0000) >> 16, (color_b & 0x00ff00) >> 8, color_b & 0x0000ff};
	double factor = (double)blend / 255;
	const uint8_t c[3] = {
		(a[0] * (1 - factor)) + (b[0] * factor),
		(a[1] * (1 - factor)) + (b[1] * factor),
		(a[2] * (1 - factor)) + (b[2] * factor),
	};
	return (c[0] << 16) + (c[1] << 8) + c[2];
}

void draw_pixel(AVFrame* frame, int x, int y, int color) {
	frame->data[0][y * frame->linesize[0] + x] = hexcode_to_ycbcr(COLOR_Y, color);
	frame->data[1][(y / 2) * frame->linesize[1] + (x / 2)] = hexcode_to_ycbcr(COLOR_CB, color);
	frame->data[2][(y / 2) * frame->linesize[2] + (x / 2)] = hexcode_to_ycbcr(COLOR_CR, color);
}

void draw_box(AVFrame *frame, Rect* r, int fg, int bg) {
	for (uint32_t y = 0; y < frame->height; y++) {
		for (uint32_t x = 0; x < frame->width; x++) {
			draw_pixel(frame, x, y, bg);
		}
	}
	if (r == NULL)
		return;

	for (uint32_t y = r->y; y < r->height; y++) {
		for (uint32_t x = r->x; x < r->width; x++) {
			draw_pixel(frame, x, y, fg);
		}
	}
}

