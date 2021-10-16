#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavfilter/avfilter.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "video.h"
#include "drawing.h"

#define PIX_FMT AV_PIX_FMT_YUV420P

// enum color_type {
// 	COLOR_Y,
// 	COLOR_CB,
// 	COLOR_CR,
// };
// static uint8_t rgb_to_ycbcr(enum color_type type, uint8_t r, uint8_t g, uint8_t b) {
// 	switch (type) {
// 	case COLOR_Y:
// 		return 16 + (((r << 6) + (r << 1) + (g << 7) + g
// 		+ (b << 4) + (b << 3) + b) >> 8);
// 	case COLOR_CB:
// 		return 128 + (((-((r << 5) + (r << 2) + (r << 1)) -
// 				((g << 6) + (g << 3)) + (g << 1)) + 
// 				(b << 7) - (b << 4)) >> 8);
// 	case COLOR_CR:
// 		return 128 + (((r << 7) - (r << 4) -
// 				((g << 6) + (g << 5) - (g << 1)) -
// 				((b << 4) + (b << 1))) >> 8);

// 	default:
// 		return 0;
// 	}
// }

// static void fill_yuv_image(AVFrame *frame, int pts) {
// 	int i;
// 	i = pts;
// 	const int r = 16, g = 188, b = 180;
// 	/* Y */
// 	for (size_t y = 0; y < frame->height; y++)
// 		for (size_t x = 0; x < frame->width; x++)
// 			frame->data[0][y * frame->linesize[0] + x] = rgb_to_ycbcr(COLOR_Y, r, g, b);
// 	/* Cb and Cr */
// 	for (size_t y = 0; y < frame->height / 2; y++) {
// 		for (size_t x = 0; x < frame->width / 2; x++) {
// 			frame->data[1][y * frame->linesize[1] + x] = rgb_to_ycbcr(COLOR_CB, r, g, b);
// 	frame->data[2][y * frame->linesize[2] + x] = rgb_to_ycbcr(COLOR_CR, r, g, b);
// 		}
// 	}
// }

// typedef struct {
// 	int x;
// 	int y;
// 	int width;
// 	int height;
// } Box;
// typedef struct {
// 	uint8_t r, g, b;
// } Color;
// uint8_t color_to_ycbcr(enum color_type type, Color* c) {
// 	return rgb_to_ycbcr(type, c->r, c->g, c->b);
// }
// typedef struct {
// 	Color bg, fg;
// 	Box b;
// } DrawOpts;
// static void draw_box(AVFrame* frame, int pts, void* data) {
// 	DrawOpts* opts = data;
// 	int x, y, i;
// 	for (y = 0; y < frame->height; y++)
// 		for (x = 0; x < frame->width; x++)
// 			frame->data[0][y * frame->linesize[0] + x] = color_to_ycbcr(COLOR_Y, &opts->bg);

// 	for (y = 0; y < frame->height / 2; y++) {
// 		for (x = 0; x < frame->width / 2; x++) {
// 			frame->data[1][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CB, &opts->bg);
// 			frame->data[2][y * frame->linesize[2] + x] = color_to_ycbcr(COLOR_CR, &opts->bg);
// 		}
// 	}

// 	for (y = opts->b.y; y < opts->b.height; y++)
// 		for (x = opts->b.x; x < opts->b.width; x++)
// 			frame->data[0][y * frame->linesize[0] + x] = color_to_ycbcr(COLOR_Y, &opts->fg);

// 	for (y = opts->b.y / 2; y < opts->b.height / 2; y++) {
// 		for (x = opts->b.x / 2; x < opts->b.width / 2; x++) {
// 			frame->data[1][y * frame->linesize[1] + x] = color_to_ycbcr(COLOR_CB, &opts->fg);
// 			frame->data[2][y * frame->linesize[2] + x] = color_to_ycbcr(COLOR_CR, &opts->fg);
// 		}
// 	}
// }

// static int write_video(const char *video_path, void (*draw_func)(AVFrame* frame, int pts, void* data), void* data) {
// 	const int FPS = 24.0;
// 	const int RES[2] = {1366, 768};
// 	const long DURATION = 1;

// 	long next_pts = 0;
// 	int samples_count = 0;
// 	float t = 0;
// 	float tincr = 0;
// 	float tincr2 = 0;

// 	AVFormatContext *fmt_ctx;
// 	avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, video_path);
// 	if (!fmt_ctx) {
// 		fprintf(stderr, "Not a recognized video format\n");
// 		return 0;
// 	}
// 	AVOutputFormat *out_fmt = fmt_ctx->oformat;
// 	if (out_fmt->video_codec == AV_CODEC_ID_NONE) {
// 		fprintf(stderr, "Not a valid video format\n");
// 		return 0;
// 	}
// 	AVCodec *cod = avcodec_find_encoder(out_fmt->video_codec);
// 	if (!cod) {
// 		fprintf(stderr, "Could not find an encoder for '%s'\n",
// 				avcodec_get_name(out_fmt->video_codec));
// 		return 0;
// 	}
// 	AVPacket *pkt = av_packet_alloc();
// 	if (!pkt) {
// 		fprintf(stderr, "Failed to allocate packet\n");
// 		return 0;
// 	}
// 	AVStream *stream = avformat_new_stream(fmt_ctx, NULL);
// 	if (!stream) {
// 		fprintf(stderr, "Could not allocate stream\n");
// 		return 0;
// 	}
// 	stream->id = fmt_ctx->nb_streams - 1;
// 	AVCodecContext *cod_ctx = avcodec_alloc_context3(cod);
// 	if (!cod_ctx) {
// 		fprintf(stderr, "Could not allocate a codec context\n");
// 		return 0;
// 	}
// 	if (cod->type != AVMEDIA_TYPE_VIDEO) {
// 		fprintf(stderr, "Codec must be video\n");
// 		return 0;
// 	}
// 	cod_ctx->codec_id = out_fmt->video_codec;
// 	cod_ctx->bit_rate = 4e5;
// 	cod_ctx->width = RES[0];
// 	cod_ctx->height = RES[1];
// 	stream->time_base = (AVRational){1, FPS};
// 	cod_ctx->time_base = stream->time_base;
// 	cod_ctx->gop_size = 12;
// 	cod_ctx->pix_fmt = PIX_FMT;

// 	switch (cod_ctx->codec_id) {
// 	case AV_CODEC_ID_MPEG2VIDEO:
// 		cod_ctx->max_b_frames = 2;
// 		break;
// 	case AV_CODEC_ID_MPEG1VIDEO:
// 		cod_ctx->mb_decision = 2;
// 		break;
// 	default:
// 		break;
// 	}

// 	// TODO can we remove this?
// 	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
// 		cod_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

// 	if (avcodec_open2(cod_ctx, cod, NULL) < 0) {
// 		fprintf(stderr, "Could not open video codec\n");
// 		return 0;
// 	}
// 	AVFrame *frame = av_frame_alloc();
// 	if (!frame) {
// 		fprintf(stderr, "Could not allocate frame\n");
// 		return 0;
// 	}
// 	frame->format = AV_PIX_FMT_GBRP;
// 	frame->width = RES[0];
// 	frame->height = RES[1];
// 	if (av_frame_get_buffer(frame, 0) < 0) {
// 		fprintf(stderr, "Could not allocate frame data\n");
// 		return 0;
// 	}
// 	if (avcodec_parameters_from_context(stream->codecpar, cod_ctx) < 0) {
// 		fprintf(stderr, "Could not copy the stream parameters\n");
// 		return 0;
// 	}

// 	// TODO should we remove this?
// 	av_dump_format(fmt_ctx, 0, video_path, 1);
// 	if (!(out_fmt->flags & AVFMT_NOFILE)) {
// 		if (avio_open(&fmt_ctx->pb, video_path, AVIO_FLAG_WRITE) < 0) {
// 			fprintf(stderr, "Could not open file %s\n", video_path);
// 			return 0;
// 		}
// 	}
// 	if (avformat_write_header(fmt_ctx, NULL) < 0) {
// 		fprintf(stderr, "Could not open output file\n");
// 		return 0;
// 	}
// 	int encode_video = 1;
// 	while (encode_video) {
// 		// get_video_frame()
// 		// AVFrame* frame2;
// 		if (av_compare_ts(next_pts, cod_ctx->time_base, DURATION,
// 						  (AVRational){1, 1}) > 0) {
// 			fprintf(stderr, "\n\nVIDEO HAS ENDED\n\n");
// 			break;
// 		}
// 		if (av_frame_make_writable(frame) < 0) {
// 			fprintf(stderr, "Frame not writable, exiting now");
// 			return 0;
// 		}
// 		// assert(cod_ctx->pix_fmt == AV_PIX_FMT_GBRP);

// 		(*draw_func)(frame, next_pts, data);

// 		frame->pts = next_pts++;

// 		int ret;
// 		if ((ret = avcodec_send_frame(cod_ctx, frame)) < 0) {
// 			fprintf(stderr, "Failed to send frame to encoder: %s\n",
// 					av_err2str(ret));
// 			return 0;
// 		}
// 		while (ret >= 0) {
// 			ret = avcodec_receive_packet(cod_ctx, pkt);
// 			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
// 				break;
// 			else if (ret < 0) {
// 				fprintf(stderr, "Failed to encode a frame: %s\n",
// 						av_err2str(ret));
// 				return 0;
// 			}
// 			av_packet_rescale_ts(pkt, cod_ctx->time_base, stream->time_base);
// 			pkt->stream_index = stream->index;
// 			ret = av_interleaved_write_frame(fmt_ctx, pkt);
// 			if (ret < 0) {
// 				fprintf(stderr, "Could not write frame to file: %s\n",
// 						av_err2str(ret));
// 				return 0;
// 			}
// 		}
// 		encode_video = ret != AVERROR_EOF;
// 	}
// 	av_write_trailer(fmt_ctx);
// 	avcodec_free_context(&cod_ctx);
// 	av_frame_free(&frame);
// 	av_packet_free(&pkt);
// 	if (!(out_fmt->flags & AVFMT_NOFILE))
// 		avio_closep(&fmt_ctx->pb);
// 	avformat_free_context(fmt_ctx);
// 	return 1;
// }

int main(int argc, char *argv[]) {
	// if (argc < 2) {
	// 	fprintf(stderr, "Need file\n");
	// 	return 1;
	// }
	VideoContext* vc = video_context_create("out.mp4", 600, 400, 24, 1);
	for (int i = 0; i < 1000; i++) {
		draw_box(vc->frame, (Rect){10, 10, 40, 40}, (Color){0, 0, 0}, (Color){255, 255, 255});
		video_context_write_frame(vc);
	}
	video_context_save_and_delete(vc);
}
