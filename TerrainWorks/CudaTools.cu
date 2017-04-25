#include "CudaTools.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_gl_interop.h"

#include "Terrain.h"

__device__ float noise(float x, float y)
{
	int n = int(x + (y + 62394) * 57);
	n = (n << 13) ^ n;
	return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589)
		& 0x7fffffff) / 1073741824.0);
}

__device__ float cosineInterpolate(float a, float b, float x)
{
	float ft = x * 3.1415927;
	float f = (1 - cos(ft)) * 0.5;

	return  a*(1 - f) + b*f;
}

__device__ float perlinHeight(float x, float y)
{
	float X = floor(x), Y = floor(y);

	float v1 = noise(X, Y);
	float v2 = noise(X + 1, Y);
	float v3 = noise(X, Y + 1);
	float v4 = noise(X + 1, Y + 1);

	return cosineInterpolate(
		cosineInterpolate(v1, v2, x - X),
		cosineInterpolate(v3, v4, x - X),
		y - Y
	);
}

__global__ void perlinNoise(float * d_heightmap, float f, float fd, float a, float ad, int iterations, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		float frequency = f;
		float amplitude = a;

		float x = ind % size, y = ind / size;

		float h = 0;

		for (int i = 0; i < iterations; i++)
		{
			h += perlinHeight(x / frequency, y / frequency)*amplitude;
			frequency /= fd;
			amplitude /= ad;

		}

		d_heightmap[ind] = h;
	}
}

__global__ void cuda_initArray(float * arr, float height, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		arr[ind] = height;
	}
}

__global__ void cuda_sqr(float * arr, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		arr[ind] = arr[ind] * arr[ind];
	}
}

__global__ void cuda_MapNormals(float * heightmap, float * normalmap, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		float x = ind % size, y = ind / size;
		bool neighbours[4];
		neighbours[0] = y < size - 1;
		neighbours[1] = y > 0;
		neighbours[2] = x < size - 1;
		neighbours[3] = x > 0;

		float nh[4];

		if (neighbours[0])
			nh[0] = heightmap[ind + size];
		else
			nh[0] = heightmap[ind];
		if (neighbours[1])
			nh[1] = heightmap[ind - size];
		else
			nh[1] = heightmap[ind];
		if (neighbours[2])
			nh[2] = heightmap[ind + 1];
		else
			nh[2] = heightmap[ind];
		if (neighbours[3])
			nh[3] = heightmap[ind - 1];
		else
			nh[3] = heightmap[ind];

		vec3 n = normalize(vec3((nh[2] - nh[3]) / 2, 1, (nh[0] - nh[1]) / 2));

		normalmap[ind * 3] = n.x;
		normalmap[ind * 3 + 1] = n.y;
		normalmap[ind * 3 + 2] = n.z;
	}
}

__global__ void cuda_elevate(float *heightmap, float offsetX, float offsetY, float centerX, float centerY, float outerRadius, float innerRadius, float factor, int innerSize, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < innerSize*innerSize)
	{
		int x = ind % innerSize + offsetX;
		int y = ind / innerSize + offsetY;
		if (x < 0 || y < 0)
			return;
		if (x >= size || y >= size)
			return;

		float dist = sqrtf((centerX - x)*(centerX - x) + (centerY - y)*(centerY - y));
		if (dist > outerRadius)
			return;

		if (dist < innerRadius)
		{
			heightmap[x*size + y] += factor;
			return;
		}
		heightmap[x*size + y] += factor * (outerRadius - dist) / (outerRadius - innerRadius);
	}
}

__global__ void cuda_averagize(float *heightmap, float *heightBuffer, float offsetX, float offsetY, float centerX, float centerY, float outerRadius, float innerRadius, float factor, int innerSize, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < innerSize*innerSize)
	{
		int x = ind % innerSize + offsetX;
		int y = ind / innerSize + offsetY;
		if (x < 0 || y < 0)
			return;
		if (x >= size || y >= size)
			return;

		float dist = sqrtf((centerX - x)*(centerX - x) + (centerY - y)*(centerY - y));

		if (dist > outerRadius)
			return;

		float deltaHeight = (heightmap[ind - 1] + heightmap[ind + 1] + heightmap[ind - size] + heightmap[ind + size]) / 4 - heightmap[ind];

		if (dist < innerRadius)
		{
			heightmap[x*size + y] += factor * deltaHeight;
			return;
		}
		ind = x*size + y;

		heightBuffer[x*size + y] = factor * deltaHeight * (outerRadius - dist) / (outerRadius - innerRadius);
	}
}

__global__ void cuda_plateao(float *heightmap, float height, float offsetX, float offsetY, float centerX, float centerY, float outerRadius, float innerRadius, float factor, bool above, bool below, int innerSize, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < innerSize*innerSize)
	{
		unsigned int x = ind % innerSize + offsetX;
		int y = ind / innerSize + offsetY;
		if (x < 0 || y < 0)
			return;
		if (x >= size || y >= size)
			return;

		float dist = sqrtf((centerX - x)*(centerX - x) + (centerY - y)*(centerY - y));
		if (dist > outerRadius)
			return;

		float delta = factor * (height - heightmap[x*size + y]);

		if (delta < 0 && !above)
			return;
		if (delta > 0 && !below)
			return;

		if (dist < innerRadius)
		{
			heightmap[x*size + y] += delta;
			return;
		}
		float fade = (outerRadius - dist) / (outerRadius - innerRadius);

		heightmap[x*size + y] += delta * fade;
	}
}

__global__ void cuda_addBuffer(float *heightmap, float *heightBuffer, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		heightmap[ind] += heightBuffer[ind];
		heightBuffer[ind] = 0;
	}
}


CudaTools::CudaTools(Terrain &terrain)
{
	cudaGLSetGLDevice(0);

	_terrain = &terrain;
	_size = terrain._size;
	N = _size*_size;
	M = 128;



	cudaGLRegisterBufferObject(terrain.vbo[1]);
	cudaGLRegisterBufferObject(terrain.vbo[2]);

	cudaGraphicsGLRegisterBuffer(&cuda_vb_resources[0], terrain.vbo[1], cudaGraphicsMapFlagsNone); // heightmap
	cudaGraphicsGLRegisterBuffer(&cuda_vb_resources[1], terrain.vbo[2], cudaGraphicsMapFlagsNone); // normals

	size_t s[2];
	s[0] = sizeof(GLfloat) * _size*_size;
	s[1] = sizeof(GLfloat) * _size*_size * 3;


	cudaGraphicsMapResources(2, cuda_vb_resources, 0);

	cudaGraphicsResourceGetMappedPointer((void**)&d_heightmap, &s[0], cuda_vb_resources[0]);
	cudaGraphicsResourceGetMappedPointer((void**)&d_normals, &s[1], cuda_vb_resources[1]);

	cudaGraphicsUnmapResources(2, cuda_vb_resources, 0);

	cudaMalloc((void**)&d_heightBuffer, sizeof(float) * _size * _size);

	setHeight(0.0f);
	fetchHeight();
	mapNormals();
}

CudaTools::~CudaTools()
{
	cudaGLUnregisterBufferObject(_terrain->vbo[1]);
	cudaGLUnregisterBufferObject(_terrain->vbo[2]);

	cudaFree(d_heightBuffer);
}

void CudaTools::setHeight(float height)
{
	cuda_initArray << <(N + M - 1) / M, M >> > ((float*)d_heightmap, height, _size);

	cudaDeviceSynchronize();
}

void CudaTools::square()
{
	cuda_sqr << <(N + M - 1) / M, M >> > ((float*)d_heightmap, _size);

	cudaDeviceSynchronize();
}

void CudaTools::PerlinNoise(float frequency, float frequencyDivider, float amplitude, float amplitudeDivider, int iterations)
{
	perlinNoise << <(N + M - 1) / M, M >> >(d_heightmap, frequency, frequencyDivider, amplitude, amplitudeDivider, iterations, _size);

	cudaDeviceSynchronize();
}

void CudaTools::mapNormals()
{
	cuda_MapNormals << < (N + M - 1) / M, M >> >(d_heightmap, d_normals, _size);
	cudaDeviceSynchronize();

}

void CudaTools::fetchHeight()
{
	cudaMemcpy(_terrain->heightmap, d_heightmap, _size *_size * sizeof(float), cudaMemcpyDeviceToHost);
}

void CudaTools::elevate(const vec2 &position, float outerRadius, float innerRadius, float factor)
{
	int _x = floor(position.x - outerRadius);
	int _y = floor(position.y - outerRadius);

	int size = ceil(2 * outerRadius) + 1;

	int n = size*size;

	cuda_elevate << <(n + M - 1) / M, M >> >(d_heightmap, _x, _y, position.x, position.y, outerRadius, innerRadius, factor, size, _size);
	cudaDeviceSynchronize();
}

void CudaTools::averagize(const vec2 &position, float outerRadius, float innerRadius, float factor)
{
	int _x = floor(position.x - outerRadius);
	int _y = floor(position.y - outerRadius);

	int size = ceil(2 * outerRadius) + 1;

	int n = size*size;

	cuda_averagize << <(n + M - 1) / M, M >> >(d_heightmap, d_heightBuffer, _x, _y, position.x, position.y, outerRadius, innerRadius, factor, size, _size);
	cudaDeviceSynchronize();
	cuda_addBuffer << <(N + M - 1) / M, M >> >(d_heightmap, d_heightBuffer, _size);
	cudaDeviceSynchronize();
}

void CudaTools::plateau(const vec3 &position, float outerRadius, float innerRadius, float factor, bool above, bool below)
{
	if (!(above || below))
		return;

	int _x = floor(position.x - outerRadius);
	int _y = floor(position.y - outerRadius);

	int size = ceil(2 * outerRadius) + 1;

	int n = size*size;

	cuda_plateao << <(n + M - 1) / M, M >> >(d_heightmap, position.z, _x, _y, position.x, position.y, outerRadius, innerRadius, factor, above, below, size, _size);
	cudaDeviceSynchronize();
}


