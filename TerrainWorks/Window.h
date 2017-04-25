#pragma once
#include <stdio.h>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

class Window
{
	GLFWwindow * window;
	int m_width;
	int m_height;
public:
	Window(int width = 1280, int height = 720);
	~Window();
	bool update();
	GLFWwindow * ptr();
	int width() const;
	int height() const;
};

