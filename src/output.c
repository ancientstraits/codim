#include "output.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"

OutputContext* output_context_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo) {

    OutputContext* oc = calloc(1, sizeof *oc);
    avformat_alloc_output_context2(&oc->fc, NULL, NULL, filename);

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

#warning TODO
void output_context_destroy(OutputContext *oc) {
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
    }

    if (oc->has_video) {
        avcodec_open2(oc->vcc, oc->vc, NULL);
        oc->vf = create_video_frame(oc->vcc);
        avcodec_parameters_from_context(oc->vs->codecpar, oc->vcc);
    }

    av_dump_format(oc->fc, 0, NULL, 1);
}

void output_context_encode(OutputContext* oc, OutputType ot) {
    if (ot == OUTPUT_TYPE_AUDIO) {
        avcodec_send_frame(oc->vcc, oc->vf);
        
    }
}
