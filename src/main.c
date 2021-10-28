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
	VideoContext* vc = video_context_create(argv[1], 600, 400, 24, 0);
	draw_box(vc->frame, &(Rect){0}, 0x000000, 0x000000);
	TextContext* tc = text_context_init("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10, 600, 600);
	draw_text(tc, vc->frame, "Hello World!", 100, 100, 0xffffff, 0x000000);
	text_context_delete(tc);
	for (int i = 0; i < 100; i++) {
		video_context_write_frame(vc);
	}
	video_context_save_and_delete(vc);
}
