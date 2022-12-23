#include <stdio.h>
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#define LOG(...) do { \
	fprintf(stderr, "%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
} while(0)

#define FAIL(...) do { LOG("Error: " __VA_ARGS__); exit(1); } while(0)

#define ASSERT(cond, ...) if (!(cond)) FAIL(__VA_ARGS__)

#define GL() do { \
	GLenum err = glGetError(); \
	ASSERT(err == GL_NO_ERROR, "OpenGL error: %d", err); \
} while(0)


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [image]", argv[0]);
        return 1;
    }
    int img_w, img_h, img_ch;
    uint8_t* img = stbi_load(argv[1], &img_w, &img_h, &img_ch, 4);

	ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWwindow* win = glfwCreateWindow(img_w, img_h, "GL Texture", NULL, NULL);
	ASSERT(win, "Failed to create GLFW window");
	glfwMakeContextCurrent(win);

    stbi_image_free(img);
    return 0;
}
