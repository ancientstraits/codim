#include <stdlib.h>
#include "output.h"

OutputContext* output_context_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo) {

    OutputContext* oc = calloc(1, sizeof *oc);
    avformat_alloc_output_context2(&oc->fc, NULL, NULL, filename);

    oc->filename = filename;

    // !! just makes it 1 or 0, just because
    oc->has_audio = !!ao;
    oc->has_video = !!vo;

    if (oc->has_audio) {
        oc->ac = avcodec_find_encoder(oc->fc->oformat->audio_codec);
        oc->ap = av_packet_alloc();
        oc->as = avformat_new_stream(oc->fc, NULL);
        oc->as->id = oc->fc->nb_streams - 1;

        oc->acc = avcodec_alloc_context3(oc->ac);
        oc->acc->sample_fmt     = AV_SAMPLE_FMT_FLTP;
        oc->acc->sample_rate    = ao->sample_rate;
        oc->acc->channels       = av_get_channel_layout_nb_channels(oc->acc->channel_layout);
        oc->acc->channel_layout = AV_CH_LAYOUT_STEREO;
        oc->as->time_base       = (AVRational){1, oc->acc->sample_rate};
    }

    if (oc->has_video) {
        oc->vc = avcodec_find_encoder(oc->fc->oformat->video_codec);
        oc->vp = av_packet_alloc();
        oc->vs = avformat_new_stream(oc->fc, NULL);
        oc->vs->id = oc->fc->nb_streams - 1;

        oc->vcc = avcodec_alloc_context3(oc->vc);
        oc->vcc->width     = vo->width;
        oc->vcc->height    = vo->height;
        oc->vcc->time_base = (AVRational){1, vo->fps};
        oc->vcc->pix_fmt   = AV_PIX_FMT_YUV420P;
    }

    return oc;
}

void output_context_destroy(OutputContext *oc) {
	avcodec_free_context(&oc->vcc);
	av_frame_free(&oc->vf);
	av_packet_free(&oc->vp);

	avcodec_free_context(&oc->acc);
	av_frame_free(&oc->afd);
	av_frame_free(&oc->af);
	av_packet_free(&oc->ap);
	swr_free(&oc->aconv);

	if (!(oc->fc->oformat->flags & AVFMT_NOFILE))
		avio_closep(&oc->fc->pb);

	avformat_free_context(oc->fc);

    free(oc);
}

static AVFrame* create_audio_frame(AVCodecContext* acc,
    enum AVSampleFormat sample_fmt, int nb_samples) {

    AVFrame* ret = av_frame_alloc();

    ret->format         = sample_fmt;
    ret->channel_layout = acc->channel_layout;
    ret->sample_rate    = acc->sample_rate;
    ret->nb_samples     = nb_samples;
    av_frame_get_buffer(ret, 0);

    return ret;
}

static SwrContext* create_swr_context(AVCodecContext* acc) {
    SwrContext* ret = swr_alloc();

	av_opt_set_int(ret, "in_channel_count", acc->channels, 0);
	av_opt_set_int(ret, "in_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(ret, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(ret, "out_channel_count", acc->channels, 0);
	av_opt_set_int(ret, "out_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(ret, "out_sample_fmt", acc->sample_fmt, 0);
	swr_init(ret);

    return ret;
}

static AVFrame* create_video_frame(AVCodecContext* vcc) {

    AVFrame* ret = av_frame_alloc();

    ret->format = vcc->pix_fmt;
    ret->width  = vcc->width;
    ret->height = vcc->height;
    av_frame_get_buffer(ret, 0);

    return ret;
}

void output_context_open(OutputContext* oc) {
    if (oc->has_audio) {
        avcodec_open2(oc->acc, oc->ac, NULL);
        if (oc->ac->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
            oc->anbs = 10000;
        else
            oc->anbs = oc->acc->frame_size;
        oc->afd = create_audio_frame(oc->acc, AV_SAMPLE_FMT_S16,  oc->anbs);
        oc->af  = create_audio_frame(oc->acc, AV_SAMPLE_FMT_FLTP, oc->anbs);
        avcodec_parameters_from_context(oc->as->codecpar, oc->acc);

        oc->aconv = create_swr_context(oc->acc);

        oc->aenc = 1;
    }

    if (oc->has_video) {
        avcodec_open2(oc->vcc, oc->vc, NULL);
        oc->vf = create_video_frame(oc->vcc);
        avcodec_parameters_from_context(oc->vs->codecpar, oc->vcc);

        oc->venc = 1;
    }

    av_dump_format(oc->fc, 0, oc->filename, 1);

    if (!(oc->fc->oformat->flags & AVFMT_NOFILE))
        avio_open(&oc->fc->pb, oc->filename, AVIO_FLAG_WRITE);

    if (avformat_write_header(oc->fc, NULL) < 0)
        exit(1);
}

int output_context_is_open(OutputContext* oc) {
    return oc->aenc || oc->venc;
}

OutputType output_context_get_encode_type(OutputContext* oc) {
    return (oc->venc && (!oc->aenc || av_compare_ts(
        oc->vpts, oc->vcc->time_base, oc->apts, oc->acc->time_base
    ) <= 0)) ? OUTPUT_TYPE_VIDEO : OUTPUT_TYPE_AUDIO;
}

double output_context_get_seconds(OutputContext* oc) {
    if (oc->has_audio)
        return av_q2d(oc->acc->time_base) * oc->apts;
    else
        return av_q2d(oc->vcc->time_base) * oc->vpts;
}

static int write_frame(AVFormatContext* fc,
    AVCodecContext* cc, AVStream* s, AVFrame* f, AVPacket* p) {

    avcodec_send_frame(cc, f);

    int ret;
    while ((ret = avcodec_receive_packet(cc, p)) >= 0) {
        av_packet_rescale_ts(p, cc->time_base, s->time_base);
        p->stream_index = s->index;
        ret = av_interleaved_write_frame(fc, p);
    }

    return ret != AVERROR_EOF;
}

void output_context_encode_video(OutputContext* oc) {
    if (oc->vf)
        oc->vf->pts = oc->vpts++;

    oc->venc = write_frame(oc->fc, oc->vcc, oc->vs, oc->vf, oc->vp);
}

void output_context_encode_audio(OutputContext* oc) {
    if (oc->vf) {
        oc->afd->pts = oc->apts;
        oc->apts += oc->anbs;

        int anbsd = av_rescale_rnd(
            swr_get_delay(oc->aconv, oc->acc->sample_rate) + oc->anbs,
            oc->acc->sample_rate, oc->acc->sample_rate, AV_ROUND_UP
        );
        swr_convert(oc->aconv, oc->af->data, anbsd,
            (const uint8_t**)oc->afd->data, oc->anbs
        );
        oc->af->pts = av_rescale_q(
            oc->asc,
            (AVRational){1, oc->acc->sample_rate},
            oc->acc->time_base
        );
        oc->asc += anbsd;
    }

    oc->aenc = write_frame(oc->fc, oc->acc, oc->as, oc->af, oc->ap);
}

void output_context_close(OutputContext* oc) {
    oc->vf = NULL;
    oc->af = NULL;

    // flush everything so that all the frames are written
    while (output_context_is_open(oc)) {
        if (output_context_get_encode_type(oc) == OUTPUT_TYPE_VIDEO)
            output_context_encode_video(oc);
        else
            output_context_encode_audio(oc);
    }

    av_write_trailer(oc->fc);
}
