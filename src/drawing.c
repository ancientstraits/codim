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

uint8_t color_to_ycbcr(int type, Color* c) {
	return rgb_to_ycbcr(type, c->r, c->g, c->b);
}

void draw_box(AVFrame *frame, Rect r, Color fg, Color bg) {
	for (uint32_t y = 0; y < frame->height; y++) {
		for (uint32_t x = 0; x < frame->width; x++) {
			frame->data[0][y * frame->linesize[0] + x] = color_to_ycbcr(COLOR_Y, &bg);
		}
	}
	for (uint32_t y = 0; y < frame->height / 2; y++) {
		for (uint32_t x = 0; x < frame->width / 2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CB, &bg);
			frame->data[2][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CR, &bg);
		}
	}

	for (uint32_t y = r.y; y < r.height; y++) {
		for (uint32_t x = r.x; x < r.width; x++) {
			frame->data[0][y * frame->linesize[0] + x] = color_to_ycbcr(COLOR_Y, &fg);
		}
	}
	for (uint32_t y = r.y / 2; y < r.height / 2; y++) {
		for (uint32_t x = r.x / 2; x < r.width / 2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CB, &fg);
			frame->data[2][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CR, &fg);
		}
	}
}

