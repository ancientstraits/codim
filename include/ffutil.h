#ifndef FFUTIL_H
#define FFUTIL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

typedef struct OutputStream {
	AVStream *st;
	AVCodecContext *enc;

	int64_t next_pts;
	int samples_count;

	AVFrame *frame;
	AVFrame *tmp_frame;

	float t, tincr, tincr2;

	struct SwsContext *sws_ctx;
	struct SwrContext *swr_ctx;
} OutputStream;

void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);
int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
					   AVStream *st, AVFrame *frame);
void add_stream(OutputStream *ost, AVFormatContext *oc,
					   const AVCodec **codec, enum AVCodecID codec_id);
AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
								  uint64_t channel_layout, int sample_rate,
								  int nb_samples);
void open_audio(AVFormatContext *oc, const AVCodec *codec,
					   OutputStream *ost, AVDictionary *opt_arg);
AVFrame *get_audio_frame(OutputStream *ost);
int write_audio_frame(AVFormatContext *oc, OutputStream *ost);
AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width,
							  int height);
void open_video(AVFormatContext *oc, const AVCodec *codec,
					   OutputStream *ost, AVDictionary *opt_arg);
void fill_yuv_image(AVFrame *pict, int frame_index,
                           int width, int height);
AVFrame *get_video_frame(OutputStream *ost);
int write_video_frame(AVFormatContext *oc, OutputStream *ost);
void close_stream(AVFormatContext *oc, OutputStream *ost);

#endif