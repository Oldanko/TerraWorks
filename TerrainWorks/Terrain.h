#pragma once
#include <GL\glew.h>
#include <glm\glm.hpp>

class CudaTools;

class Terrain
{
	friend class CudaTools;

	GLuint _size;
	float * heightmap;

	GLuint vbo[3];
	GLuint ebo;

	GLuint * indexicate();
	GLfloat * getGrid();
public:
	Terrain(GLuint size);
	~Terrain();

	float getHeight(float _x, float _y);
	void bindVboGrid();
	void bindVboHeightmap();
	void bindVboNormals();
	void bindEbo();
};

