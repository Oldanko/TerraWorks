#pragma once
#include <glm\glm.hpp>
typedef unsigned short ushort;
typedef unsigned char ubyte;
typedef char byte;

class Window;

const ubyte CAMERA_MOVE_FORWARD		= 0b00000001;
const ubyte CAMERA_MOVE_BACK		= 0b00000010;
const ubyte CAMERA_MOVE_LEFT		= 0b00000100;
const ubyte CAMERA_MOVE_RIGHT		= 0b00001000;


template<class T>struct KeyifPressed
{
	int m_id;
	T m_action;
	KeyifPressed(int id, T action) : m_id(id), m_action(action) {}
};

template<class T>struct KeyPressed
{
	int m_id;
	T m_action;
	bool isPressed;
	KeyPressed(int id, T action) : m_id(id), m_action(action) {}
};


class Controls
{
	static double m_MMBx, m_MMBy;
	static double m_x, m_y;
	static double m_dX, m_dY;
	static Window *window;
	static KeyifPressed<ubyte>* m_cameraControlsKeys;
	static ubyte m_camera;
	static bool m_isMMBPressed;

public:
	static void init(Window *window); // read from file
	static ubyte cameraControls();
	static void update();

	static bool isMMBPressed();

	static float x();
	static float y();
	static float dx();
	static float dy();
};

