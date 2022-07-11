#include "display.hh"
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

using namespace std;

int display(vector<string> const&) {
	struct glfwHandle {
		glfwHandle() {
			if (!glfwInit())
				throw std::runtime_error("glfwHandle");
		}
		~glfwHandle() {
			glfwTerminate();
		}
	}
	use_glfw;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 960, "Hello World", nullptr, nullptr);

	if (!window)
		return -1;

	glfwMakeContextCurrent(window);

	if (gl3wInit())
		return -1;

	cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n' << std::flush;

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
