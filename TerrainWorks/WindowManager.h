#pragma once
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

class WindowManager
{
	static GLFWwindow *m_window;

	static int m_height;
	static int m_width;

	static float m_lastTime;
	static float m_timeDelta;

	static glm::mat4 m_projectionMatrix;
public:
	static void init();
	static bool update();
	static void close();

	static const int& width();
	static const int& height();
	static GLFWwindow *window();
	static const float& timeDelta();
	static const glm::mat4 projectionMatrix();
};

