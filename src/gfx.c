#include "gfx.h"
#include "error.h"

#include <stdlib.h>

#include <epoxy/egl.h>
#include <epoxy/gl.h>

#define GDIE(...) DIE(ERROR_GFX, __VA_ARGS__)
#define GASSERT(cond, ...) if(!(cond)) GDIE(__VA_ARGS__)

#ifdef __linux__
struct GfxContextInternal {
	EGLDisplay dpy;
	EGLContext ctx;
	EGLSurface surf;
};

static GfxContextInternal* gci_create(int width, int height) {
	GfxContextInternal* gci = calloc(1, sizeof *gci);

	gci->dpy = eglGetDisplay(NULL);
	eglInitialize(gci->dpy, NULL, NULL);
	eglBindAPI(EGL_OPENGL_API);

	EGLint conf_attr[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_CONFORMANT, EGL_OPENGL_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_LUMINANCE_SIZE, 0,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_LEVEL, 0,
        EGL_BUFFER_SIZE, 24,
        EGL_NONE
	};
	int num_conf;
	EGLConfig conf;
	eglChooseConfig(gci->dpy, conf_attr, &conf, 1, &num_conf);
	GASSERT(num_conf >= 1, "No EGL configs found");

	EGLint pbuf_attr[] = {
		EGL_WIDTH, width,
		EGL_HEIGHT, height,
		EGL_NONE
	};
	gci->surf = eglCreatePbufferSurface(gci->dpy, conf, pbuf_attr);

	gci->ctx = eglCreateContext(gci->dpy, conf, NULL, NULL);
	GASSERT(gci->ctx != EGL_NO_CONTEXT, "Could not create EGL context");

	GASSERT(eglMakeCurrent(gci->dpy, gci->surf, gci->surf, gci->ctx), "Could not set the EGL context");

	return gci;
}

static void gci_destroy(GfxContextInternal* gci) {
	eglDestroyContext(gci->dpy, gci->ctx);
	eglDestroySurface(gci->dpy, gci->surf);
	eglTerminate(gci->dpy);
	free(gci);
}

static void gci_render(GfxContextInternal* gci, uint8_t* buf, int width, int height) {
	eglSwapBuffers(gci->dpy, gci->surf);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
}
#endif // __linux__

GfxContext* gfx_create(int width, int height) {
	GfxContext* gc = calloc(1, sizeof *gc);
	gc->gci = gci_create(width, height);
	return gc;
}

void gfx_render(GfxContext* gc, uint8_t* buf) {
	gci_render(gc->gci, buf, gc->width, gc->height);
}

void gfx_destroy(GfxContext* gc) {
	gci_destroy(gc->gci);
	free(gc);
}

