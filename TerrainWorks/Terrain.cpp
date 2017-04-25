#include "Terrain.h"

float linearInterpolaton(float a, float b, float x)
{
	return a*(1 - x) + b*x;
}

Terrain::Terrain(GLuint size)
{
	_size = size;
	glGenBuffers(5, vbo);

	GLfloat * grid = getGrid();
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, size * size * 2 * sizeof(GLfloat), grid, GL_STATIC_DRAW); // grid
	delete[] grid;

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, size * size * sizeof(GLfloat), nullptr, GL_DYNAMIC_COPY); // heightmap

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, size * size * 3 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW); // normals

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, size * size * sizeof(GLfloat), nullptr, GL_DYNAMIC_COPY); // watermap

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, size * size * 3 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW); // waternormals

	glGenBuffers(1, &ebo);
	GLuint * indices = indexicate();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (size - 1)*(size - 1) * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	delete[] indices;

	glGenVertexArrays(2, vao);
	glBindVertexArray(vao[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);


	glBindVertexArray(vao[1]);


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	heightmap = new float[_size*_size];
}

Terrain::~Terrain()
{
	glDeleteVertexArrays(2, vao);
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


void Terrain::drawTerrain() const
{
	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, (_size - 1)*(_size - 1) * 6, GL_UNSIGNED_INT, 0);
}

void Terrain::drawWater() const
{
	glBindVertexArray(vao[1]);
	glDrawElements(GL_TRIANGLES, (_size - 1)*(_size - 1) * 6, GL_UNSIGNED_INT, 0);
}