#pragma once
#include <glm\glm.hpp>
using namespace glm;

struct POV
{
	mat4 matrix;
	vec3 position;
	vec3 direction;
};

class Camera
{
	POV m_pov;
	float m_angleX;
	float m_angleY;
public:
	void update();
	Camera();
	Camera(float x, float y, float z, float angleX, float angleY);
	Camera(vec3 position, float angleX, float angleY);
	~Camera();

	const POV& pov() const;
	const mat4& matrix() const;
	const vec3& position() const;
	float angleX() const;
	float angleY() const;
};

