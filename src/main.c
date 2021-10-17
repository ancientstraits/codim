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
	VideoContext* vc = video_context_create(argv[1], 600, 400, 24, 1);
	TextContext* tc  = text_context_init("/Users/newuser/file.ttf", 5, 600, 400);
	draw_box(vc->frame, (Rect){50, 50, 525, 325}, 0x333333, 0xCCCCCC);
	draw_text(tc, vc->frame, "File: src/main.c", 50, 25, 0x000000);
	draw_text(tc, vc->frame, string, 100, 75, 0xFFFFFF);
	for (int i = 0; i < 1000; i++) {
		video_context_write_frame(vc);
	}
	text_context_delete(tc);
	video_context_save_and_delete(vc);
}
