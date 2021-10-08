#include <libavcodec/packet.h>
#include <libavformat/avio.h>
#include <libavutil/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavfilter/avfilter.h>

static void fill_yuv_image(AVFrame *frame, int pts) {
	int x, y, i;
	i = pts;
	/* Y */
	for (y = 0; y < frame->height; y++)
		for (x = 0; x < frame->width; x++)
			frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
	/* Cb and Cr */
	for (y = 0; y < frame->height / 2; y++) {
		for (x = 0; x < frame->width / 2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
			frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
		}
	}
}

static int write_video(const char *video_path, void (*draw_func)(AVFrame* frame, int pts)) {
	const int FPS = 24.0;
	const int RES[2] = {1366, 768};
	const long DURATION = 10;

	long next_pts = 0;
	int samples_count = 0;
	float t = 0;
	float tincr = 0;
	float tincr2 = 0;

	AVFormatContext *fmt_ctx;
	avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, video_path);
	if (!fmt_ctx) {
		fprintf(stderr, "Not a recognized video format\n");
		return 0;
	}
	AVOutputFormat *out_fmt = fmt_ctx->oformat;
	if (out_fmt->video_codec == AV_CODEC_ID_NONE) {
		fprintf(stderr, "Not a valid video format\n");
		return 0;
	}
	AVCodec *cod = avcodec_find_encoder(out_fmt->video_codec);
	if (!cod) {
		fprintf(stderr, "Could not find an encoder for '%s'\n",
				avcodec_get_name(out_fmt->video_codec));
		return 0;
	}
	AVPacket *pkt = av_packet_alloc();
	if (!pkt) {
		fprintf(stderr, "Failed to allocate packet\n");
		return 0;
	}
	AVStream *stream = avformat_new_stream(fmt_ctx, NULL);
	if (!stream) {
		fprintf(stderr, "Could not allocate stream\n");
		return 0;
	}
	stream->id = fmt_ctx->nb_streams - 1;
	AVCodecContext *cod_ctx = avcodec_alloc_context3(cod);
	if (!cod_ctx) {
		fprintf(stderr, "Could not allocate a codec context\n");
		return 0;
	}
	if (cod->type != AVMEDIA_TYPE_VIDEO) {
		fprintf(stderr, "Codec must be video\n");
		return 0;
	}
	cod_ctx->codec_id = out_fmt->video_codec;
	cod_ctx->bit_rate = 4e5;
	cod_ctx->width = RES[0];
	cod_ctx->height = RES[1];
	stream->time_base = (AVRational){1, FPS};
	cod_ctx->time_base = stream->time_base;
	cod_ctx->gop_size = 12;
	cod_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	switch (cod_ctx->codec_id) {
	case AV_CODEC_ID_MPEG2VIDEO:
		cod_ctx->max_b_frames = 2;
		break;
	case AV_CODEC_ID_MPEG1VIDEO:
		cod_ctx->mb_decision = 2;
		break;
	default:
		break;
	}

	// TODO can we remove this?
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		cod_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (avcodec_open2(cod_ctx, cod, NULL) < 0) {
		fprintf(stderr, "Could not open video codec\n");
		return 0;
	}
	AVFrame *frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate frame\n");
		return 0;
	}
	frame->format = AV_PIX_FMT_YUV420P;
	frame->width = RES[0];
	frame->height = RES[1];
	if (av_frame_get_buffer(frame, 0) < 0) {
		fprintf(stderr, "Could not allocate frame data\n");
		return 0;
	}
	if (avcodec_parameters_from_context(stream->codecpar, cod_ctx) < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		return 0;
	}

	// TODO should we remove this?
	av_dump_format(fmt_ctx, 0, video_path, 1);
	if (!(out_fmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&fmt_ctx->pb, video_path, AVIO_FLAG_WRITE) < 0) {
			fprintf(stderr, "Could not open file %s\n", video_path);
			return 0;
		}
	}
	if (avformat_write_header(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not open output file\n");
		return 0;
	}
	int encode_video = 1;
	while (encode_video) {
		// get_video_frame()
		// AVFrame* frame2;
		if (av_compare_ts(next_pts, cod_ctx->time_base, DURATION,
						  (AVRational){1, 1}) > 0) {
			fprintf(stderr, "\n\nVIDEO HAS ENDED\n\n");
			break;
		}
		if (av_frame_make_writable(frame) < 0) {
			fprintf(stderr, "Frame not writable, exiting now");
			return 0;
		}
		assert(cod_ctx->pix_fmt == AV_PIX_FMT_YUV420P);

		(*draw_func)(frame, next_pts);

		frame->pts = next_pts++;

		// write_frame()
		int ret;
		if ((ret = avcodec_send_frame(cod_ctx, frame)) < 0) {
			fprintf(stderr, "Failed to send frame to encoder: %s\n",
					av_err2str(ret));
			return 0;
		}
		while (ret >= 0) {
			ret = avcodec_receive_packet(cod_ctx, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			else if (ret < 0) {
				fprintf(stderr, "Failed to encode a frame: %s\n",
						av_err2str(ret));
				return 0;
			}
			av_packet_rescale_ts(pkt, cod_ctx->time_base, stream->time_base);
			pkt->stream_index = stream->index;
			ret = av_interleaved_write_frame(fmt_ctx, pkt);
			if (ret < 0) {
				fprintf(stderr, "Could not write frame to file: %s\n",
						av_err2str(ret));
				return 1;
			}
		}
		encode_video = ret != AVERROR_EOF;
	}
	av_write_trailer(fmt_ctx);
	avcodec_free_context(&cod_ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	if (!(out_fmt->flags & AVFMT_NOFILE))
		avio_closep(&fmt_ctx->pb);
	avformat_free_context(fmt_ctx);
	return 1;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Need file\n");
		return 1;
	}
	return !write_video(argv[1], fill_yuv_image);
}
