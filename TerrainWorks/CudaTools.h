#pragma once
#include <GL\glew.h>
#include <glm\glm.hpp>

using namespace glm;

class Terrain;

class CudaTools
{
	Terrain *_terrain;
	GLuint _size;
	float * d_heightmap;
	float * d_heightBuffer;
	float * d_normals;

	GLuint N;
	GLuint M;

	struct cudaGraphicsResource *cuda_vb_resources[2];
public:
	CudaTools(Terrain &terrain);
	~CudaTools();

	void setHeight(float height);
	void square();
	void PerlinNoise(float frequency, float frequencyDivider, float amplitude, float amplitudeDivider, int iterations);
	
	void mapNormals();
	void fetchHeight();

	void elevate(const vec2 &position, float outerRadius, float innerRadius, float factor);
	void averagize(const vec2& position, float outerRadius, float innerRadius, float factor);
	void plateau(const vec3 &position, float outerRadius, float innerRadius, float factor, bool above, bool below);
};

