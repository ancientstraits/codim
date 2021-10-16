#ifndef VIDEO_H
#define VIDEO_H

#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct {
	size_t duration;
	size_t pts;
	AVFormatContext *fmt_ctx;
	// AVOutputFormat *out_fmt;
	AVCodec *cod;
	AVCodecContext *cod_ctx;
	AVStream *stream;
	AVPacket *pkt;
	AVFrame *frame;
} VideoContext;

// Allocate a new VideoContext and set its default values.
VideoContext *video_context_create(const char *video_path, size_t width,
								   size_t height, size_t fps, size_t duration);
int video_context_should_update(VideoContext *vc);

int video_context_write_frame(VideoContext *vc);
// Delete a VideoContext.
void video_context_save_and_delete(VideoContext *vc);

#endif