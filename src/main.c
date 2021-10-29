#include <stdio.h>
#include <stdlib.h>

#include "video.h"
#include "drawing.h"
#include "text.h"
#include "filter.h"
#include "fcutil.h"

#define PIX_FMT AV_PIX_FMT_YUV420P

#define FFERR(fn) \
	if (fn < 0) \
		fprintf(stderr, "FFERR @ %d\n", __LINE__)

#define mk(struct_name, ...) (struct_name) {__VA_ARGS__}


const char* STR = 
	"#include <stdio.h>\n"
	"int main(int argc, char* argv[]) {\n"
	"    printf(\"Hello World!\\n\")\n"
	"    return 0;\n"
	"}"
;

int SPEED = 2;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [OUTPUT_VIDEO_FILE]\n", argv[0]);
		return 1;
	}
	VideoContext* vc = video_context_create(argv[1], 600, 400, 24, 0);
	fill_frame(vc->frame, 0x000000);

	char* str = findMonospaceFont();
	TextContext* tc = text_context_init(str, 20, 600, 600);
	free(str);

	// TODO is `+1` needed?
	char iter[strlen(STR) + 1];

	size_t cursor_loc[2] = {20, 20};

	for (size_t i = 0; STR[i] != '\0'; i++) {
		iter[i] = STR[i];
		iter[i + 1] = '\0';
		draw_text(tc, vc->frame, iter, 20, 20, 0xffffff, 0x000000);
		Rect r = {0, 0, 100, 100};
		draw_box(vc->frame, &r, 0xffffff);
		printf("cursor_loc = {%lu, %lu}\n", cursor_loc[0], cursor_loc[1]);
		draw_box(vc->frame, &r, 0xffffff);
		for (int j = 0; j < SPEED; j++) {
			video_context_write_frame(vc);
		}
	}
	
	printf("final iter:\n%s\n", iter);

	for (int i = 0; i < 20; i++) {
		video_context_write_frame(vc);
	}

	text_context_delete(tc);
	video_context_save_and_delete(vc);
}
