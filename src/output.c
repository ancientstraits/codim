#include "output.h"
#include "error.h"
#include "libavutil/error.h"

#define ODIE(...) DIE(ERROR_OUTPUT, __VA_ARGS__)

#define ODIE_ALLOCATE_ERR(...) \
    ODIE("Could not allocate " __VA_ARGS__)

// Initialize the members of an audio or video codec.
// Either the audio or video codec variables can be passed to this function.
static void init_codec(
    enum AVCodecID cid,
    AVFormatContext* fc,
    const AVCodec** c,
    AVPacket** p,
    AVStream** s,
    AVCodecContext** cc
) {
    *c = avcodec_find_encoder(cid);
    if (!c) ODIE("Could not find encoder for %s", avcodec_get_name(cid));

    *p = av_packet_alloc();
    if (!*p) ODIE_ALLOCATE_ERR("AVPacket");

    *s = avformat_new_stream(fc, NULL);
    if (!*s) ODIE_ALLOCATE_ERR("AVStream");
    (*s)->id = fc->nb_streams - 1;

    *cc = avcodec_alloc_context3(*c);
    if (!*cc) ODIE_ALLOCATE_ERR("AVCodecContext");
}

OutputContext* output_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo) {

    int ret;

    OutputContext* oc = calloc(1, sizeof *oc);
    if (!oc) DIE_ERRNO(ERROR_ALLOC);

    ret = avformat_alloc_output_context2(&oc->fc, NULL, NULL, filename);
    if (ret < 0) ODIE_ALLOCATE_ERR("AVFormatContext: %s", av_err2str(ret));

    oc->filename = filename;

    // !! just makes it 1 or 0, just because
    oc->has_audio = !!ao;
    oc->has_video = !!vo;

    if (oc->has_audio) {
        init_codec(oc->fc->oformat->audio_codec,
            oc->fc, &oc->ac, &oc->ap, &oc->as, &oc->acc);

        oc->acc->sample_fmt     = AV_SAMPLE_FMT_FLTP;
        oc->acc->sample_rate    = ao->sample_rate;
        oc->acc->channels       = av_get_channel_layout_nb_channels(oc->acc->channel_layout);
        oc->acc->channel_layout = AV_CH_LAYOUT_STEREO;
        oc->as->time_base       = (AVRational){1, oc->acc->sample_rate};
    }

    if (oc->has_video) {
        init_codec(oc->fc->oformat->video_codec,
            oc->fc, &oc->vc, &oc->vp, &oc->vs, &oc->vcc);

        oc->vcc->width     = vo->width;
        oc->vcc->height    = vo->height;
        oc->vcc->time_base = (AVRational){1, vo->fps};
        oc->vcc->pix_fmt   = AV_PIX_FMT_YUV420P;
    }

    return oc;
}

// Returns an allocated video frame with properties taken from `acc`.
// @param acc - the context to examine
static AVFrame* create_audio_frame(AVCodecContext* acc,
    enum AVSampleFormat sample_fmt, int nb_samples) {

    AVFrame* f = av_frame_alloc();
    if (!f) ODIE_ALLOCATE_ERR("AVFrame");

    f->format         = sample_fmt;
    f->channel_layout = acc->channel_layout;
    f->sample_rate    = acc->sample_rate;
    f->nb_samples     = nb_samples;
    int errnum = av_frame_get_buffer(f, 0);
    if (errnum < 0) ODIE_ALLOCATE_ERR("audio data buffer: %s", av_err2str(errnum));

    return f;
}

// Returns an allocated `SwrContext*` with properties taken from `acc`..
// @param acc - the context to examine
static SwrContext* create_swr_context(AVCodecContext* acc) {
    SwrContext* ret = swr_alloc();
    if (!ret) ODIE_ALLOCATE_ERR("SwrContext");

	av_opt_set_int(ret, "in_channel_count", acc->channels, 0);
	av_opt_set_int(ret, "in_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(ret, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(ret, "out_channel_count", acc->channels, 0);
	av_opt_set_int(ret, "out_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(ret, "out_sample_fmt", acc->sample_fmt, 0);
	int errnum = swr_init(ret);
    if (errnum < 0) ODIE("Could not initialize SwrContext: %s", av_err2str(errnum));

    return ret;
}

// Returns an allocated video frame with properties taken from `vcc`.
// @param vcc - the context to examine
static AVFrame* create_video_frame(AVCodecContext* vcc) {
    AVFrame* f = av_frame_alloc();
    if (!f) ODIE_ALLOCATE_ERR("AVFrame");

    f->format = vcc->pix_fmt;
    f->width  = vcc->width;
    f->height = vcc->height;
    int errnum = av_frame_get_buffer(f, 0);
    if (errnum < 0) ODIE_ALLOCATE_ERR("video data buffer: %s", av_err2str(errnum));

    return f;
}

void output_open(OutputContext* oc) {
    int errnum;
    if (oc->has_audio) {
        errnum = avcodec_open2(oc->acc, oc->ac, NULL);
        if (errnum < 0) ODIE("Could not open audio codec: %s", av_err2str(errnum));

        if (oc->ac->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
            oc->anbs = 10000;
        else
            oc->anbs = oc->acc->frame_size;
        oc->afd = create_audio_frame(oc->acc, AV_SAMPLE_FMT_S16,  oc->anbs);
        oc->af  = create_audio_frame(oc->acc, AV_SAMPLE_FMT_FLTP, oc->anbs);

        errnum = avcodec_parameters_from_context(oc->as->codecpar, oc->acc);
        if (errnum < 0) ODIE("Could not copy parameters: %s", av_err2str(errnum));

        oc->aconv = create_swr_context(oc->acc);

        oc->aenc = 1;
    }

    if (oc->has_video) {
        avcodec_open2(oc->vcc, oc->vc, NULL);
        if (errnum < 0) ODIE("Could not open video codec: %s", av_err2str(errnum));

        oc->vf = create_video_frame(oc->vcc);
        avcodec_parameters_from_context(oc->vs->codecpar, oc->vcc);

        oc->venc = 1;
    }

    av_dump_format(oc->fc, 0, oc->filename, 1);

    if (!(oc->fc->oformat->flags & AVFMT_NOFILE)) {
        errnum = avio_open(&oc->fc->pb, oc->filename, AVIO_FLAG_WRITE);
        if (errnum < 0) ODIE("Could not run `avio_open()`: %s", av_err2str(errnum));
    }

    errnum = avformat_write_header(oc->fc, NULL);
    if (errnum < 0) ODIE("Could not write header: %s", av_err2str(errnum));
}

int output_is_open(OutputContext* oc) {
    return oc->aenc || oc->venc;
}

OutputType output_get_encode_type(OutputContext* oc) {
    return (oc->venc && (!oc->aenc || av_compare_ts(
        oc->vpts, oc->vcc->time_base, oc->apts, oc->acc->time_base
    ) <= 0)) ? OUTPUT_TYPE_VIDEO : OUTPUT_TYPE_AUDIO;
}

double output_get_seconds(OutputContext* oc) {
    if (oc->has_audio)
        return av_q2d(oc->acc->time_base) * oc->apts;
    else
        return av_q2d(oc->vcc->time_base) * oc->vpts;
}

// Writes a single frame using `fc`, `cc`, `s`, `f`, and `p`.
static int write_frame(AVFormatContext* fc,
    AVCodecContext* cc, AVStream* s, AVFrame* f, AVPacket* p) {

    int errnum;

    errnum = avcodec_send_frame(cc, f);
    if (errnum < 0) ODIE("Could not send frame to encoder: %s", av_err2str(errnum));

    while ((errnum = avcodec_receive_packet(cc, p)) >= 0) {
        av_packet_rescale_ts(p, cc->time_base, s->time_base);
        p->stream_index = s->index;
        errnum = av_interleaved_write_frame(fc, p);
    }

    return errnum != AVERROR_EOF;
}

void output_encode_video(OutputContext* oc) {
    if (oc->vf)
        oc->vf->pts = oc->vpts++;

    oc->venc = write_frame(oc->fc, oc->vcc, oc->vs, oc->vf, oc->vp);
}

void output_encode_audio(OutputContext* oc) {
    int errnum;

    if (!oc->vf)
        goto end;

    oc->afd->pts = oc->apts;
    oc->apts += oc->anbs;

    int anbsd = av_rescale_rnd(
        swr_get_delay(oc->aconv, oc->acc->sample_rate) + oc->anbs,
        oc->acc->sample_rate, oc->acc->sample_rate, AV_ROUND_UP
    );

    errnum = swr_convert(oc->aconv, oc->af->data, anbsd,
        (const uint8_t**)oc->afd->data, oc->anbs
    );
    if (errnum < 0) ODIE("Could not convert data: %s", av_err2str(errnum));

    oc->af->pts = av_rescale_q(
        oc->asc,
        (AVRational){1, oc->acc->sample_rate},
        oc->acc->time_base
    );
    oc->asc += anbsd;

end:
    oc->aenc = write_frame(oc->fc, oc->acc, oc->as, oc->af, oc->ap);
}

void output_close(OutputContext* oc) {
    int errnum;

    oc->vf = NULL;
    oc->af = NULL;

    // flush everything so that all the frames are written
    while (output_is_open(oc)) {
        if (output_get_encode_type(oc) == OUTPUT_TYPE_VIDEO)
            output_encode_video(oc);
        else
            output_encode_audio(oc);
    }

    errnum = av_write_trailer(oc->fc);
    if (errnum < 0) ODIE("Could not write trailer: %s", av_err2str(errnum));
}

void output_destroy(OutputContext *oc) {
    int errnum;

    if (oc->has_audio) {
        avcodec_free_context(&oc->acc);
        av_frame_free(&oc->afd);
        av_frame_free(&oc->af);
        av_packet_free(&oc->ap);
        swr_free(&oc->aconv);
    }

    if (oc->has_video) {
        avcodec_free_context(&oc->vcc);
        av_frame_free(&oc->vf);
        av_packet_free(&oc->vp);
    }

	if (!(oc->fc->oformat->flags & AVFMT_NOFILE)) {
		errnum = avio_closep(&oc->fc->pb);
        if (errnum < 0) ODIE("Could not run `avio_close()`: %s", av_err2str(errnum));
    }

	avformat_free_context(oc->fc);

    free(oc);
}
