#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "video.h"

VideoContext *video_context_create(const char *video_path, size_t width,
								   size_t height, size_t fps, size_t duration) {
	VideoContext *vc = malloc(sizeof(VideoContext));
	avformat_alloc_output_context2(&vc->fmt_ctx, NULL, NULL, video_path);
    if (!vc->fmt_ctx) {
        fprintf(stderr, "Not a recognized video format\n");
        return NULL;
    }
    AVOutputFormat* out_fmt = vc->fmt_ctx->oformat;
    if (out_fmt->video_codec == AV_CODEC_ID_NONE) {
        fprintf(stderr, "Not a recognized video format\n");
        return NULL;
    }
    vc->cod = avcodec_find_encoder(out_fmt->video_codec);
    if (!vc->cod) {
        fprintf(stderr, "Could not find an encoder for %s\n",
            avcodec_get_name(out_fmt->video_codec));
		return NULL;
    }
	vc->pkt = av_packet_alloc();
	if (!vc->pkt) {
		fprintf(stderr, "Failed to allocate packet\n");
		return NULL;
	}
	vc->stream = avformat_new_stream(vc->fmt_ctx, NULL);
	if (!vc->stream) {
		fprintf(stderr, "Failed to allocate stream\n");
		return NULL;
	}
	vc->stream->id = vc->fmt_ctx->nb_streams;
	vc->cod_ctx = avcodec_alloc_context3(vc->cod);
	if (!vc->cod_ctx) {
		fprintf(stderr, "Failed to allocate codec context\n");
		return NULL;
	}
	if (vc->cod->type != AVMEDIA_TYPE_VIDEO) {
		fprintf(stderr, "Codec is not video\n");
		return NULL;
	}
	vc->cod_ctx->codec_id = out_fmt->video_codec;
	// TODO try changing this value
	vc->cod_ctx->bit_rate = 4e5;
	vc->cod_ctx->width = width;
	vc->cod_ctx->height = height;
	vc->stream->time_base = (AVRational){1, fps};
	vc->cod_ctx->time_base = vc->stream->time_base;
	// TODO what does this value do?
	vc->cod_ctx->gop_size = 12;
	vc->cod_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	// TODO is this even important?
	switch (vc->cod_ctx->codec_id) {
	case AV_CODEC_ID_MPEG2VIDEO:
		vc->cod_ctx->max_b_frames = 2;
		break;
	case AV_CODEC_ID_MPEG1VIDEO:
		vc->cod_ctx->mb_decision = 2;
		break;
	default:
		break;
	}

	// TODO can we remove this?
	if (vc->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		vc->cod_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	if (avcodec_open2(vc->cod_ctx, vc->cod, NULL) < 0) {
		fprintf(stderr, "Failed to open video codec\n");
		return NULL;
	}
	vc->frame = av_frame_alloc();
	if (!vc->frame) {
		fprintf(stderr, "Failed to allocate frame\n");
		return NULL;
	}
	// TODO can this be changed?
	vc->frame->format = AV_PIX_FMT_YUV420P;
	vc->frame->width = width;
	vc->frame->height = height;
	if (av_frame_get_buffer(vc->frame, 0) < 0)  {
		fprintf(stderr, "Failed to allocate frame data\n");
		return NULL;
	}
	if (avcodec_parameters_from_context(vc->stream->codecpar, vc->cod_ctx) < 0) {
		fprintf(stderr, "Failed to copy stream parameters\n");
		return NULL;
	}

	// TODO should we remove this?
	av_dump_format(vc->fmt_ctx, 0, video_path, 1);
	// TODO can we use OR instead of !?
	if (!(out_fmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&vc->fmt_ctx->pb, video_path, AVIO_FLAG_WRITE) < 0) {
			fprintf(stderr, "Failed to open file %s\n", video_path);
			return NULL;
		}
	}
	if (avformat_write_header(vc->fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Failed to open output file\n");
		return NULL;
	}
	vc->pts = 0;
	return vc;
}

int video_context_should_update(VideoContext *vc) {
	return !(av_compare_ts(vc->pts, vc->cod_ctx->time_base, vc->duration, (AVRational){1, 1}) < 0);
}


// TODO should there be an av_frame_make_writable()?
int video_context_write_frame(VideoContext *vc) {
	int ret;
	// TODO can we remove vc->pts?
	vc->frame->pts = vc->pts++;
	printf("PTS = %lu\n", vc->pts);
	if ((ret = avcodec_send_frame(vc->cod_ctx, vc->frame)) < 0) {
		fprintf(stderr, "Failed to send frame to encoder: %s\n", av_err2str(ret));
		return 0;
	}
	while (ret >= 0) {
		ret = avcodec_receive_packet(vc->cod_ctx, vc->pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			fprintf(stderr, "Failed to encode a frame: %s\n", av_err2str(ret));
			return 0;
		}
		// TODO do we need this?
		av_packet_rescale_ts(vc->pkt, vc->cod_ctx->time_base, vc->stream->time_base);
		vc->pkt->stream_index = vc->stream->index;
		ret = av_interleaved_write_frame(vc->fmt_ctx, vc->pkt);
		if (ret < 0) {
			fprintf(stderr, "Failed to write frame to file: %s\n", av_err2str(ret));
			return 0;
		}
	}
	return ret != AVERROR_EOF;
}

// TODO should this return an int?
void video_context_save_and_delete(VideoContext *vc) {
	av_write_trailer(vc->fmt_ctx);
	avcodec_free_context(&vc->cod_ctx);
	av_frame_free(&vc->frame);
	av_packet_free(&vc->pkt);
    AVOutputFormat* out_fmt = vc->fmt_ctx->oformat;
	if (!(out_fmt->flags & AVFMT_NOFILE))
		avio_closep(&vc->fmt_ctx->pb);
	avformat_free_context(vc->fmt_ctx);
	free(vc);
}
