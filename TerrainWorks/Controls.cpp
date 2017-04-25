#include "Controls.h"
#include "Window.h"

KeyifPressed<ubyte> cameraControls_default[] = {
	KeyifPressed<ubyte>(GLFW_KEY_W,			CAMERA_MOVE_FORWARD),
	KeyifPressed<ubyte>(GLFW_KEY_S,			CAMERA_MOVE_BACK),
	KeyifPressed<ubyte>(GLFW_KEY_A,			CAMERA_MOVE_LEFT),
	KeyifPressed<ubyte>(GLFW_KEY_D,			CAMERA_MOVE_RIGHT)
};

KeyifPressed<ubyte>* Controls::m_cameraControlsKeys = cameraControls_default;

ubyte Controls::m_camera = 0;

Window *Controls::window = nullptr;
double Controls::m_x;
double Controls::m_y;
double Controls::m_MMBx;
double Controls::m_MMBy;
double Controls::m_dX;
double Controls::m_dY;
bool Controls::m_isMMBPressed = false;


void Controls::init(Window *window)
{	
	Controls::window = window;
}
ubyte Controls::cameraControls()
{
	return m_camera;
}
void Controls::update()
{
	if(!window)
		return;
	// =================Keyboard Input=================

	//Camera Controls
	m_camera = 0;

	for (int i = 0; i < 4; i++) // 
		if (glfwGetKey(window->ptr(), m_cameraControlsKeys[i].m_id) == GLFW_PRESS)
			m_camera |= m_cameraControlsKeys[i].m_action;

	// =================Mouse Input====================
	glfwGetCursorPos(window->ptr(), &m_x, &m_y);

	if (glfwGetMouseButton(window->ptr(), GLFW_MOUSE_BUTTON_MIDDLE))
	{
		if (!m_isMMBPressed)
		{
			m_isMMBPressed = true;
			m_MMBx = m_x;
			m_MMBy = m_y;
		}

		m_dX = m_MMBx - m_x;
		m_dY = m_MMBy - m_y;

		glfwSetCursorPos(window->ptr(), m_MMBx, m_MMBy);
	}
	else
		m_isMMBPressed = false;

}

bool Controls::isMMBPressed()
{
	return m_isMMBPressed;
}

float Controls::x()
{
	return m_x;
}

float Controls::y()
{
	return m_y;
}

float Controls::dx()
{
	return m_dX;
}

float Controls::dy()
{
	return m_dY;
}
