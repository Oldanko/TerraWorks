#include "Window.h"


Window::Window(int width, int height)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return;
	}

	m_width = width;
	m_height = height;

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "OpenGL Engine", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		window = nullptr;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

}

Window::~Window()
{
	glfwSetWindowShouldClose(window, GL_TRUE);
}

bool Window::update()
{
	glfwSwapBuffers(window);
	glfwPollEvents();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	return !glfwWindowShouldClose(window);
}

GLFWwindow * Window::ptr()
{
	return window;
}

int Window::width() const
{
	return m_width;
}

int Window::height() const
{
	return m_height;
}
