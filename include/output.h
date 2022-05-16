#ifndef CODIM_OUTPUT_H
#define CODIM_OUTPUT_H

// One more time, I won't use global state for this.
// But if this doesn't work, I'm embracing global variables.

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

typedef struct OutputContext {
    int has_audio, has_video;
    AVFormatContext* fc;

    AVCodec* ac;
    AVCodecContext* acc;
    AVStream* as;
    AVPacket* ap;
    AVFrame* af;
    AVFrame* afd;
    SwrContext* aconv;
    int apts, aenc, anbs;

    AVCodec* vc;
    AVCodecContext* vcc;
    AVStream* vs;
    AVPacket* vp;
    AVFrame* vf;
    int vpts, venc;
} OutputContext;

typedef struct OutputAudioOpts {
    int sample_rate;
} OutputAudioOpts;

typedef struct OutputVideoOpts {
    int width, height, fps;
} OutputVideoOpts;

OutputContext* output_context_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo);
void output_context_open(OutputContext* oc);
void output_context_encode_audio(OutputContext* oc);
void output_context_encode_video(OutputContext* oc);
void output_context_close(OutputContext* oc);
void output_context_destroy(OutputContext* oc);

#endif
