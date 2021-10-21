#include <libavutil/frame.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavfilter/avfilter.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "video.h"
#include "drawing.h"
#include "text.h"
#include "filter.h"

#define PIX_FMT AV_PIX_FMT_YUV420P


const char* string = 
	"#include <stdio.h>\n"
	"int main(int argc, char* argv[]) {\n"
	"    printf(\"Hello World!\\n\")\n"
	"    return 0;\n"
	"}"
;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [OUTPUT_VIDEO_FILE]\n", argv[0]);
		return 1;
	}
	VideoContext* vc = video_context_create(argv[1], 1920, 1080, 24, 1);
	FilterContext* fc = filter_context_create(vc);
	AVFrame* frame = malloc(sizeof(AVFrame));
	filter_context_exec(fc, frame, "null");
	free(frame);
	video_context_write_frame(vc);
	video_context_save_and_delete(vc);
}
