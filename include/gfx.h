#ifndef CODIM_GFX_H
#define CODIM_GFX_H

#include <stdint.h>

// `gfx.h` provides an API to draw to OpenGL in a headless manner.
// The context it uses is opaque because it may differ depending on the OS.
typedef struct GfxContextInternal GfxContextInternal;

typedef struct GfxContext {
	GfxContextInternal* gci;
	int width;
	int height;
} GfxContext;

// Allocates and returns a pointer to a `GfxContext`.
// After this function is called, one should be able to use regular OpenGL draw calls.
GfxContext* gfx_create(int width, int height);
// Render and return a buffer containing the RGBA data of the image.
void gfx_render(GfxContext* gc, uint8_t* buf);
// Free the memory of a `GfxContext`.
void gfx_destroy(GfxContext* gc);

#endif // !CODIM_GFX_H

