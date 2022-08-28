#ifndef CODIM_GFX_H
#define CODIM_GFX_H

#include <stdint.h>

// `libswscale` will be used for converting OpenGL's RGB into YUV for videos.
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
typedef struct SwsContext SwsContext;

// `gfx.h` provides an API to draw to OpenGL in a headless manner.
// The context it uses is opaque because it may differ depending on the OS.
typedef struct GfxContextInternal GfxContextInternal;

typedef struct GfxContext {
	GfxContextInternal* gci;
	int width;
	int height;
	SwsContext* sc;
	uint8_t* rgb_buf;
} GfxContext;

// Allocates and returns a pointer to a `GfxContext`.
// After this function is called, one should be able to use regular OpenGL draw calls.
GfxContext* gfx_create(int width, int height);
// Renders and writes to a frame.
void gfx_render(GfxContext* gc, AVFrame* frame);
// Free the memory of a `GfxContext`.
void gfx_destroy(GfxContext* gc);

#endif // !CODIM_GFX_H

