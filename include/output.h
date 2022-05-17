#ifndef CODIM_OUTPUT_H
#define CODIM_OUTPUT_H

// `output.h` abstracts the FFmpeg C libraries
// to make it easier to write videos.

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

// A struct that holds all the state for the
// encoding of a multimedia file.
typedef struct OutputContext {
    // Base Components

    // `has_audio` and `has_video` are enabled or disabled whenever
    // the multimedia file has audio or video features, or does not.
    int has_audio, has_video;
    // `fc` is used to manage the encoding of all the streams.
    AVFormatContext* fc;
    // `filename` holds the name of the file that the `OutputContext
    // is writing to.
    const char* filename;

    // Audio Components

    // The audio codec.
    AVCodec* ac;
    // The audio codec context.
    AVCodecContext* acc;
    // The audio stream.
    AVStream* as;
    // The encoded audio packet to write to the file.
    AVPacket* ap;
    // The audio frame that uses FLTP (should not be drawn to).
    AVFrame* af;
    // The audio frame that uses S16 (should be drawn to).
    AVFrame* afd;
    // The conversion context used to convert the `afd` to the `af`.
    SwrContext* aconv;
    // The frame number.
    int apts;
    // Whether the audio stream is being encoded or not.
    int aenc;
    // The maximum number of samples per frame.
    int anbs;
    // The current samples count.
    int asc;


    // Video Components

    // The video codec.
    AVCodec* vc;
    // The video codec context.
    AVCodecContext* vcc;
    // The video stream.
    AVStream* vs;
    // The encoded video packet to write to the file.
    AVPacket* vp;
    // The raw video data that should be drawn to.
    AVFrame* vf;
    // The frame number.
    int vpts;
    // Whether the video stream is being encoded or not.
    int venc;
} OutputContext;

// A struct used to pass in audio options.
typedef struct OutputAudioOpts {
    // The sample rate to be used for the audio.
    int sample_rate;
} OutputAudioOpts;

// A struct used to pass in video options.
typedef struct OutputVideoOpts {
    // The width, height, and frames per second of the video.
    int width, height, fps;
} OutputVideoOpts;

// Returns a newly allocated `OutputContext*`.
// If `ao` or `vo` is `NULL`, then the video will not be encoded with audio/video.
// @param filename - The name of the file to write to.
// @param ao - The audio options, or `NULL` if audio is not being encoded.
// @param vo - The video options, or `NULL` if video is not being encoded.
OutputContext* output_context_create(
    const char* filename, OutputAudioOpts* ao, OutputVideoOpts* vo);

// Opens the file and writes a header to it.
// @param[inout] oc - the context to modify
void output_context_open(OutputContext* oc);
// Returns `1` if the context is currently open, or `0` otherwise.
// @param[inout] oc - the context to modify
int output_context_is_open(OutputContext* oc);

double output_context_get_seconds(OutputContext* oc);

typedef enum OutputType {
    OUTPUT_TYPE_AUDIO,
    OUTPUT_TYPE_VIDEO,
} OutputType;
OutputType output_context_get_encode_type(OutputContext* oc);

void output_context_encode_audio(OutputContext* oc);
void output_context_encode_video(OutputContext* oc);
void output_context_close(OutputContext* oc);
void output_context_destroy(OutputContext* oc);

#endif // !OUTPUT_H
