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
    const char* filename;

    AVCodec* ac;
    AVCodecContext* acc;
    AVStream* as;
    AVPacket* ap;
    AVFrame* af;
    AVFrame* afd;
    SwrContext* aconv;
    int apts, aenc, anbs, asc;

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

typedef enum OutputType {
    OUTPUT_TYPE_AUDIO,
    OUTPUT_TYPE_VIDEO,
} OutputType;

OutputContext* output_context_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo);
void output_context_open(OutputContext* oc);
int output_context_is_open(OutputContext* oc);
double output_context_get_seconds(OutputContext* oc);
OutputType output_context_get_encode_type(OutputContext* oc);
void output_context_encode(OutputContext* oc, OutputType ot);
void output_context_close(OutputContext* oc);
void output_context_destroy(OutputContext* oc);

#endif // !OUTPUT_H
