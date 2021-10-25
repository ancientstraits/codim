#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <stdio.h>
#include <stdlib.h>

#include "filter.h"

FilterContext* filter_context_create(VideoContext* vc) {
    FilterContext* fc = malloc(sizeof(FilterContext));
    fc->buffersrc = avfilter_get_by_name("buffer");
    fc->buffersink = avfilter_get_by_name("buffersink");
    fc->inputs = avfilter_inout_alloc();
    if (!fc->inputs) {
        fprintf(stderr, "Failed to allocate filter inputs\n");
        return NULL;
    }
    fc->outputs = avfilter_inout_alloc();
    if (!fc->inputs) {
        fprintf(stderr, "Failed to allocate filter inputs\n");
        return NULL;
    }
    fc->filtergraph = avfilter_graph_alloc();
    if (!fc->filtergraph) {
        fprintf(stderr, "Failed to allocate filtergraph\n");
    }

    char args[512];
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        vc->cod_ctx->width, vc->cod_ctx->height, AV_PIX_FMT_YUV420P,
        vc->cod_ctx->time_base.num, vc->cod_ctx->time_base.den,
        vc->cod_ctx->sample_aspect_ratio.num, vc->cod_ctx->sample_aspect_ratio.den);

    printf("Args:\n%s\n", args);
    
    if (avfilter_graph_create_filter(&fc->buffersrc_ctx, fc->buffersrc, "in", args, NULL, fc->filtergraph) < 0) {
        fprintf(stderr, "Failed to create buffer source\n");
        return NULL;
    }
    if (avfilter_graph_create_filter(&fc->buffersink_ctx, fc->buffersink, "out", NULL, NULL, fc->filtergraph) < 0) {
        fprintf(stderr, "Failed to create buffer sink\n");
        return NULL;
    }
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    if (av_opt_set_int_list(fc->buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_YUV420P, AV_OPT_SEARCH_CHILDREN) < 0) {
        fprintf(stderr, "Could not set pixel format\n");
        return NULL;
    }

    fc->outputs->name = av_strdup("in");
    fc->outputs->filter_ctx = fc->buffersrc_ctx;
    fc->outputs->pad_idx = 0;
    fc->outputs->next = NULL;

    fc->inputs->name = av_strdup("out");
    fc->inputs->filter_ctx = fc->buffersink_ctx;
    fc->inputs->pad_idx = 0;
    fc->inputs->next = NULL;

    return fc;
}

AVFrame* filter_context_exec(FilterContext* fc, AVFrame* frame, const char* filter_desc) {
    AVFrame* filt_frame = av_frame_alloc();
    if (!filt_frame) {
        fprintf(stderr, "Failed to allocate filtered frame\n");
        return NULL;
    }
    if (avfilter_graph_parse_ptr(fc->filtergraph, filter_desc, &fc->inputs, &fc->outputs, NULL) < 0) {
        fprintf(stderr, "Failed to parse filtergraph\n");
        return NULL;
    }
    if (avfilter_graph_config(fc->filtergraph, NULL) < 0) {
        fprintf(stderr, "Failed to configure filtergraph\n");
        return NULL;
    }

    if (av_buffersrc_add_frame_flags(fc->buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
        fprintf(stderr, "Failed to add frame to filter\n");
        return NULL;
    }
    // TODO for loop?
    if (av_buffersink_get_frame(fc->buffersink_ctx, filt_frame) < 0) {
        fprintf(stderr, "Could not get frame from filter\n");
        return NULL;
    }
    // while (1) {
    //     int ret = av_buffersink_get_frame(fc->buffersink_ctx, filt_frame);
    //     if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    //         break;
    //     if (ret < 0) {
    //         fprintf(stderr, "Frame could not be retrieved from filter\n");
    //         return NULL;
    //     }
    // }
    return filt_frame;
}

void filter_context_destroy(FilterContext *fc) {
    avfilter_inout_free(&fc->inputs);
    avfilter_inout_free(&fc->outputs);
    free(fc);
}
