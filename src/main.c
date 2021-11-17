#include <stdio.h>
#include <stdlib.h>

#include "lualang.h"
#include "video.h"
#include "drawing.h"
#include "text.h"
#include "filter.h"
#include "fcutil.h"

const char *STR = "#include <stdio.h>\n"
				  "int main(int argc, char* argv[]) {\n"
				  "    printf(\"Hello World!\\n\")\n"
				  "    return 0;\n"
				  "}";

const int SPEED = 2;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [LUA_SCRIPT]\n", argv[0]);
		return 1;
	}
	return !eval_lua_script(argv[1], "codim");
}

// int main(int argc, char *argv[]) {
// 	if (argc < 2) {
// 		fprintf(stderr, "Usage: %s [OUTPUT_VIDEO_FILE]\n", argv[0]);
// 		return 1;
// 	}
// 	VideoContext* vc = video_context_create(argv[1], 600, 400, 24, 0);
// 	fill_frame(vc->frame, 0x000000);

// 	char* str = findMonospaceFont();
// 	TextContext* tc = text_context_init(str, 20, 600, 600);
// 	free(str);

// 	tc->loc.x = 20;
// 	tc->loc.y = 40;

// 	const int fontsize = tc->face->size->metrics.height / 64;

// 	for (int i = 0; STR[i]; i++) {
// 		draw_single_char(tc, vc->frame, STR[i], 20, 40, 0xffffff, 0x000000);
// 		draw_box(vc->frame, &(Rect){tc->loc.x, tc->loc.y - fontsize, 5, fontsize},
// 0xffffff); 		for (int j = 0; j < SPEED; j++) { 			video_context_write_frame(vc);
// 		}
// 		draw_box(vc->frame, &(Rect){tc->loc.x, tc->loc.y - fontsize, 5, fontsize},
// 0x000000);
// 	}
// 	for (int i = 0; i < 20; i++) {
// 		video_context_write_frame(vc);
// 	}

// 	text_context_delete(tc);

// 	video_context_save_and_delete(vc);
// }
