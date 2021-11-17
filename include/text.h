#ifndef TEXT_H
#define TEXT_H

#include <libavcodec/avcodec.h>
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	unsigned char *buf;
	struct {
		unsigned int x;
		unsigned int y;
	} size;
	struct {
		long x;
		long y;
	} bearing;
	unsigned int advance;
	unsigned int pitch;
} FontChar;

typedef struct {
	FT_Library lib;
	FT_Face face;
	struct {
		int x;
		int y;
	} loc;
	FontChar chars[256];
} TextContext;

TextContext *text_context_init(const char *font_path, size_t font_size);
void text_context_delete(TextContext *tc);

int draw_text(TextContext *tc, AVFrame *frame, const char *str, int x, int y,
			  int fg, int bg);
int draw_single_char(TextContext *tc, AVFrame *frame, char c, int xpos,
					 int ypos, int fg, int bg);

#endif // !TEXT_H
