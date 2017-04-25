#include "Camera.h"

#include <glm\gtx\transform.hpp>
#include "Controls.h"
#include "WindowManager.h"

void Camera::update()
{
	if (Controls::isMMBPressed())
	{
		const float angleMax = 3.14f / 2;
		m_angleX += Controls::dx()* 0.005f;
		m_angleY += Controls::dy() * -0.005f;
		if (m_angleY > angleMax)
			m_angleY = angleMax;
		if (m_angleY < -angleMax)
			m_angleY = -angleMax;

	}
	m_pov.direction = mat3(rotate(m_angleX, vec3(0, 1, 0))) * vec3(1, 0, 1);
	m_pov.direction = mat3(rotate(m_angleY, normalize(vec3(m_pov.direction.z, 0, -m_pov.direction.x)))) * m_pov.direction;

	if (Controls::cameraControls() & CAMERA_MOVE_FORWARD)
		m_pov.position += m_pov.direction;
	if (Controls::cameraControls() & CAMERA_MOVE_BACK)
		m_pov.position -= m_pov.direction;

	auto forward = normalize(vec2(m_pov.direction.x, m_pov.direction.z));

	if (Controls::cameraControls() & CAMERA_MOVE_LEFT)
	{
		m_pov.position.x += forward.y;
		m_pov.position.z -= forward.x;
	}
	if (Controls::cameraControls() & CAMERA_MOVE_RIGHT)
	{
		m_pov.position.x -= forward.y;
		m_pov.position.z += forward.x;
	}
	m_pov.matrix = lookAt(m_pov.position, m_pov.position + m_pov.direction, glm::vec3(0, 1, 0));
}

Camera::Camera()
{
	m_pov.position = vec3(0, 0, 0);
	m_angleX = 0;
	m_angleY = 0;
}

Camera::Camera(float x, float y, float z, float angleX, float angleY)
{
	m_pov.position = vec3(x, y, z);
	m_angleX = angleX;
	m_angleY = angleY;
}

Camera::Camera(vec3 position, float angleX, float angleY)
{
	m_pov.position = position;
	m_angleX = angleX;
	m_angleY = angleY;
}


Camera::~Camera()
{
}

const POV& Camera::pov() const
{
	return m_pov;
}

const mat4& Camera::matrix() const
{
	return m_pov.matrix;
}

const vec3& Camera::position() const
{
	return m_pov.position;
}

float Camera::angleX() const
{
	return m_angleX;
}

float Camera::angleY() const
{
	return m_angleY;
}
