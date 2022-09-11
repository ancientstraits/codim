#include <stdio.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* win = glfwCreateWindow(600, 400, "tri", NULL, NULL);
	glfwMakeContextCurrent(win);
	
	while(!glfwWindowShouldClose(win)) {
		glClearColor(0.0, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBegin(GL_POLYGON);
			glColor3f(1, 0, 0); glVertex3f(-0.6, -0.75, 0.5);
			glColor3f(0, 1, 0); glVertex3f(0.6, -0.75, 0);
			glColor3f(0, 0, 1); glVertex3f(0, 0.75, 0);
		glEnd();
		glFlush();
		
		glfwSwapBuffers(win);
		glfwPollEvents();
	}
	glfwTerminate();
}
