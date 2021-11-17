#ifndef FILTER_H
#define FILTER_H

#include <libavfilter/avfilter.h>

#include "video.h"

typedef struct {
	AVFilterContext *buffersrc_ctx;
	const AVFilter *buffersrc;
	AVFilterContext *buffersink_ctx;
	const AVFilter *buffersink;
	AVFilterGraph *filtergraph;
	AVFilterInOut *outputs;
	AVFilterInOut *inputs;
} FilterContext;

FilterContext *filter_context_create(VideoContext *vc);
AVFrame *filter_context_exec(FilterContext *fc, AVFrame *frame,
							 const char *filter_desc);
void filter_context_destroy(FilterContext *fc);

#endif