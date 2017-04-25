#pragma once
#include <GL\glew.h>
#include <glm\glm.hpp>

class CudaTools;

class Terrain
{
	friend class CudaTools;

	GLuint _size;
	float * heightmap;

	GLuint vao[2];
	GLuint vbo[5];
	GLuint ebo;

	GLuint * indexicate();
	GLfloat * getGrid();
public:
	Terrain(GLuint size);
	~Terrain();

	float getHeight(float _x, float _y);
	void drawTerrain() const;
	void Terrain::drawWater() const;
};

