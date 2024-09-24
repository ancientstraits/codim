#include "gfx.h"
#include "error.h"
#include "scripting.h"

#include <stdlib.h>

#include <epoxy/gl.h>

#define GASSERT(cond, ...) ASSERT(cond, ERROR_GFX, __VA_ARGS__)

static void GLAPIENTRY
on_gl_err( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam ) {
	fprintf( stderr, "GL CALLBACK: %s source=0x%x type=0x%x, id=%d, severity=0x%x, message=\"%s\" pts=%d\n",
		( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
		source, type, id, severity, message, scripting_state.oc->apts);
}


#ifdef __linux__
// On Linux, EGL will be used to render the image.
#include <epoxy/egl.h>

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
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf);
}
#else
// If not on Linux, we can get away with using an invisible GLFW window instead, since the display server
// on macOs and Windows is always on.
#include <GLFW/glfw3.h>

struct GfxContextInternal {
	GLFWwindow* win;
	GLuint fbo;
	GLuint txt;
};

static GfxContextInternal* gci_create(int width, int height) {
	GfxContextInternal* gci = calloc(1, sizeof *gci);

	GASSERT(glfwInit(), "Failed to initialize GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfw window will be visible when `VISIBLE` env var exists
	glfwWindowHint(GLFW_VISIBLE, getenv("VISIBLE") ? GLFW_TRUE : GLFW_FALSE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
	gci->win = glfwCreateWindow(width, height, "Codim", NULL, NULL);
	GASSERT(gci->win, "Failed to create GLFW window");

	glfwMakeContextCurrent(gci->win);

	// int err = glewInit();
	// GASSERT(err == GLEW_OK, "Glew failed: %s", glewGetErrorString(err));

	glGenBuffers(1, &gci->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gci->fbo);

	glGenTextures(1, &gci->txt);
	glBindTexture(GL_TEXTURE_2D, gci->txt);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gci->txt, 0);

	GASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

	return gci;
}

static void gci_destroy(GfxContextInternal* gci) {
	glDeleteBuffers(1, &gci->fbo);
	glDeleteTextures(1, &gci->txt);
	glfwTerminate();
}

static void gci_render(GfxContextInternal* gci, uint8_t* buf, int width, int height) {
	glfwPollEvents();
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glfwSwapBuffers(gci->win);
	glfwPollEvents();
}
#endif

GfxContext* gfx_create(int width, int height) {
	GfxContext* gc = calloc(1, sizeof *gc);
	gc->rgb_buf = calloc(4 * width * height, 1);
	gc->width = width;
	gc->height = height;
	gc->gci = gci_create(width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(on_gl_err, NULL);

	gc->sc = sws_getContext(
		width, height, AV_PIX_FMT_RGB24,
		width, height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL
	);
	GASSERT(gc->sc, "Could not create SwsContext");
	return gc;
}

void gfx_render(GfxContext* gc, AVFrame* frame) {
	gci_render(gc->gci, gc->rgb_buf, gc->width, gc->height);

	int stride = 3 * gc->width;
	sws_scale(
		gc->sc, (const uint8_t* const*)(&gc->rgb_buf), &stride,
		0, gc->height, frame->data, frame->linesize
	);
}

void gfx_destroy(GfxContext* gc) {
	gci_destroy(gc->gci);
	sws_freeContext(gc->sc);
	free(gc->rgb_buf);
	free(gc);
}

