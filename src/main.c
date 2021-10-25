#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
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

#define FFERR(fn) \
	if (fn < 0) \
		fprintf(stderr, "FFERR @ %d\n", __LINE__)


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
	VideoContext* vc = video_context_create(argv[1], 600, 400, 24, 1);
	FilterContext* fc = filter_context_create(vc);
	draw_box(vc->frame, &(Rect){20, 20, 200, 200}, 0x0044FF, 0x00FF44);
	vc->frame = filter_context_exec(fc, vc->frame,
		"drawtext=text='Sample Text':box=1:x=20:y=20:borderw=3:bordercolor=white:fontsize=24:fontcolor=black"
	);
	for (int i = 0; i < 100; i++) {
		video_context_write_frame(vc);
	}
	filter_context_destroy(fc);
	video_context_save_and_delete(vc);
}
