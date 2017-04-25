#include "Terrain.h"

float linearInterpolaton(float a, float b, float x)
{
	return a*(1 - x) + b*x;
}

Terrain::Terrain(GLuint size)
{
	_size = size;
	glGenBuffers(3, vbo);

	GLfloat * grid = getGrid();
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, size * size * 2 * sizeof(GLfloat), grid, GL_STATIC_DRAW); // grid
	delete[] grid;

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, size * size * sizeof(GLfloat), nullptr, GL_DYNAMIC_COPY); // heightmap

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, size * size * 3 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW); // normals

	glGenBuffers(1, &ebo);
	GLuint * indices = indexicate();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size - 1)*(size - 1) * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	delete[] indices;

	heightmap = new float[_size*_size];
}

Terrain::~Terrain()
{
	glDeleteBuffers(3, vbo);
	glDeleteBuffers(1, &ebo);

	delete[] heightmap;
}

GLuint * Terrain::indexicate()
{
	unsigned int * indices = new unsigned int[(_size - 1) * (_size - 1) * 6];

	for (int i = 0; i < _size - 1; i++)
		for (int j = 0; j < _size - 1; j++)
		{
			int ind = (i * (_size - 1) + j) * 6;

			indices[ind] = i * _size + j;
			indices[ind + 1] = (i + 1) * _size + j + 1;
			indices[ind + 2] = (i + 1) * _size + j;
			indices[ind + 3] = i * _size + j;
			indices[ind + 4] = i * _size + j + 1;
			indices[ind + 5] = (i + 1) * _size + j + 1;
		}
	return indices;
}

GLfloat * Terrain::getGrid()
{
	GLfloat * grid = new GLfloat[_size *_size * 2];

	for (int i = 0; i < _size; i++)
		for (int j = 0; j < _size; j++)
		{
			int ind = i * _size + j;
			grid[ind * 2] = i;
			grid[ind * 2 + 1] = j;
		}

	return grid;
}

float Terrain::getHeight(float x, float y)
{
	float X = floor(x);
	float Y = floor(y);
	float _x = x - X;
	float _y = y - Y;

	if (X < 0 || Y < 0)
		return 0.0f;

	if (X > _size - 1 || Y > _size - 1)
		return 0.0f;

	int target = X*_size + Y;
	float a = heightmap[target];
	float b = heightmap[target + 1];
	float c = heightmap[target + _size];
	float d = heightmap[target + _size + 1];


	float result = linearInterpolaton(
		linearInterpolaton(a, b, _x),
		linearInterpolaton(c, d, _x),
		y);

	return heightmap[target];
}

void Terrain::bindVboGrid() { glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); }
void Terrain::bindVboHeightmap() { glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); }
void Terrain::bindVboNormals() { glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); }
void Terrain::bindEbo() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); }