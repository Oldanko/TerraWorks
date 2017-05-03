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
	if (ind < size)
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

__global__ void cuda_MapNormals(float * heightmap, float * normalmap, float h, int size)
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
			nh[0] = heightmap[ind + size]*h;
		else
			nh[0] = heightmap[ind] * h;
		if (neighbours[1])
			nh[1] = heightmap[ind - size] * h;
		else
			nh[1] = heightmap[ind] * h;
		if (neighbours[2])
			nh[2] = heightmap[ind + 1] * h;
		else
			nh[2] = heightmap[ind] * h;
		if (neighbours[3])
			nh[3] = heightmap[ind - 1] * h;
		else
			nh[3] = heightmap[ind] * h;

		vec3 n = normalize(vec3((nh[2] - nh[3]) / 2, 1, (nh[0] - nh[1]) / 2));

		normalmap[ind * 3] = n.x;
		normalmap[ind * 3 + 1] = n.y;
		normalmap[ind * 3 + 2] = n.z;
	}
}

__global__ void cuda_MapNormals(float * heightmap, float * watermap, float * normalmap, float h, int size)
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
			nh[0] = heightmap[ind + size] * h + watermap[ind + size];
		else
			nh[0] = heightmap[ind] * h + watermap[ind];
		if (neighbours[1])
			nh[1] = heightmap[ind - size] * h + watermap[ind - size];
		else
			nh[1] = heightmap[ind] * h + watermap[ind];
		if (neighbours[2])
			nh[2] = heightmap[ind + 1] * h + watermap[ind + 1];
		else
			nh[2] = heightmap[ind] * h + watermap[ind];
		if (neighbours[3])
			nh[3] = heightmap[ind - 1] * h + watermap[ind - 1];
		else
			nh[3] = heightmap[ind] * h + watermap[ind];

		vec3 n = normalize(vec3((nh[2] - nh[3]) / 2, 1, (nh[0] - nh[1]) / 2));

		normalmap[ind * 3] = n.x;
		normalmap[ind * 3 + 1] = n.y;
		normalmap[ind * 3 + 2] = n.z;
	}
}

__global__ void cuda_Add(float *Arr, float factor, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		Arr[ind] += factor;
		if (Arr[ind] < 0)
			Arr[ind] = 0;
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
		heightmap[x*size + y] += factor * cosineInterpolate(0, 1, (outerRadius - dist) / (outerRadius - innerRadius));
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

		int here = x*size + y;

		float dist = sqrtf((centerX - x)*(centerX - x) + (centerY - y)*(centerY - y));

		if (dist > outerRadius)
			return;

		float avg = (heightmap[here + 1] + heightmap[here - 1] + heightmap[here - size] + heightmap[here + size]) / 4;

		if (dist < innerRadius)
		{
			heightBuffer[here] = heightmap[here] * (1-factor) + avg * factor;
			return;
		}
		ind = x*size + y;

		if (outerRadius - innerRadius == 0)
			return;

		float fade = factor * cosineInterpolate(0, 1, (outerRadius - dist) / (outerRadius - innerRadius));

		heightBuffer[here] = heightmap[here] * (1-fade) + avg * fade;
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

		if (outerRadius - innerRadius == 0)
			return;

		heightmap[x*size + y] += delta * cosineInterpolate(0, 1, (outerRadius - dist) / (outerRadius - innerRadius));
	}
}

__global__ void cuda_addBuffer(float *heightmap, float *heightBuffer, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		if(heightBuffer[ind] == 0) return;
		heightmap[ind] = heightBuffer[ind];
		heightBuffer[ind] = 0;
	}
}

__global__ void waterFlowI(float *heightmap, float * watermap, float * flow, float h, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		float x = ind % size, y = ind / size;
		bool neighbours[4] =
		{
			y < size - 1,
			y > 0,
			x < size - 1,
			x > 0
		};

		int n[4] = { ind + size, ind - size, ind + 1, ind - 1 };
		int f[4] = { ind * 4 ,ind * 4 + 1, ind * 4 + 2, ind * 4 + 3 };

		auto waterlvlDelta = [heightmap, watermap, ind, h](int nind)
		{ return (heightmap[ind]*h + watermap[ind]) - (heightmap[nind]*h + watermap[nind]); };


		for (int i = 0; i < 4; i++)
		{
			if (neighbours[i])
				flow[f[i]] = max(0.0f, flow[f[i]]*0.9f + waterlvlDelta(n[i])*0.5f);
			else
				flow[f[i]] = 0;
		}

		float totalFlow = flow[f[0]] + flow[f[1]] + flow[f[2]] + flow[f[3]];
		if (totalFlow == 0)
			return;

		float K = min(1.0f,	watermap[ind] / (totalFlow));

		for (int i = 0; i < 4; i++)
		{
			flow[f[i]] = K * flow[f[i]];
		}
	}
}

__global__ void waterFlowII(float* heightmap, float* normalmap, float* sediment, float * watermap, float * flow, float * velocity, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		float x = ind % size, y = ind / size;
		bool neighbours[4] =
		{
			y < size - 1,
			y > 0,
			x < size - 1,
			x > 0
		};

		int n[4] = { ind + size, ind - size, ind + 1, ind - 1 };
		int f[4] = { ind * 4 ,ind * 4 + 1, ind * 4 + 2, ind * 4 + 3 };
		int nf[4] = { n[0]*4 + 1, n[1]*4, n[2]*4 + 3, n[3]*4 + 2 };


		float outflow = flow[f[0]] + flow[f[1]] + flow[f[2]] + flow[f[3]];
		float inflow = 0.0f;

		for (int i = 0; i < 4; i++)
			if (neighbours[i])
				inflow += flow[nf[i]];

		float waterAvg = watermap[ind];

		watermap[ind] = watermap[ind] + (inflow - outflow)*0.5f;

		waterAvg += watermap[ind];
		waterAvg /= 2;
		
		if (waterAvg == 0)
		{
			velocity[ind * 2] = 0;
			velocity[ind * 2 + 1] = 0;
		}
		else
		{
			velocity[ind * 2] = (flow[f[0]] - flow[f[1]] + flow[nf[1]] - flow[nf[0]]) / 2 / waterAvg;
			velocity[ind * 2 + 1] = (flow[f[2]] - flow[f[3]] + flow[nf[3]] - flow[nf[2]]) / 2 / waterAvg;
		}

		/*if (abs(velocity[ind * 2]) > 1 || abs(velocity[ind * 2 + 1]) > 1)
		{
			heighttmap[ind] = 0;
		}*/

		float Kc = 0.01f;
		float Ke = 0.1f;
		float Kd = 1.0f;
		float C =
			Kc *
			min(0.3f, dot(vec3(0, 1, 0), vec3(normalmap[ind * 3], normalmap[ind * 3 + 1], normalmap[ind * 3 + 2]))) *
			//abs(outflow - inflow);
			sqrt(velocity[ind * 2]* velocity[ind * 2] + velocity[ind * 2 + 1]* velocity[ind * 2 + 1]);

		float delta = 0;

		if (sediment[ind] < 0)
			heightmap[ind] = 0;


		if (C > sediment[ind])
		{//erode
			delta = C - sediment[ind];
			heightmap[ind] -= delta * Ke;
			sediment[ind] += delta * Ke;
		}
		else
		{//deposit
			delta = sediment[ind] - C;
			heightmap[ind] += delta * Kd;
			sediment[ind] -= delta  * Kd;
		}

	}
}

__global__ void waterFlowIII(float * sediment, float * sedimentBuffer, float * velocity, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		float x = ind % size, y = ind / size;
		bool neighbours[4] =
		{
			y < size - 1,
			y > 0,
			x < size - 1,
			x > 0
		};

		int n[4] = { ind + size, ind - size, ind + 1, ind - 1 };
		

		sedimentBuffer[ind] = 0;
		sedimentBuffer[ind] = (1 - abs(velocity[ind * 2]))*(1 - abs(velocity[ind * 2 + 1])) * sediment[ind] * 0.5f;


		if (neighbours[0])
			if (velocity[n[0] * 2] < 0)
				sedimentBuffer[ind] += abs(velocity[n[0] * 2])*(1 - abs(velocity[n[0] * 2 + 1])) * sediment[n[0]] * 0.5f;
		if (neighbours[1])
			if (velocity[n[1] * 2] > 0)
				sedimentBuffer[ind] += abs(velocity[n[1] * 2])*(1 - abs(velocity[n[1] * 2 + 1])) * sediment[n[1]] * 0.5f;
		if (neighbours[2])
			if (velocity[n[2] * 2 + 1] < 0)
				sedimentBuffer[ind] += (1 - abs(velocity[n[2] * 2]))*abs(velocity[n[2] * 2 + 1]) * sediment[n[2]] * 0.5f;
		if (neighbours[3])
			if (velocity[n[3] * 2 + 1] > 0)
				sedimentBuffer[ind] += (1 - abs(velocity[n[3] * 2]))*abs(velocity[n[3] * 2 + 1]) * sediment[n[3]] * 0.5f;


	}
}

__global__ void waterFlowIV(float * sediment, float * sedimentBuffer, int size)
{
	int ind = blockIdx.x * blockDim.x + threadIdx.x;
	if (ind < size*size)
	{
		sediment[ind] = sedimentBuffer[ind];
		sedimentBuffer[ind] = 0;

	}
}

//__global__ void tempWeathering(float * heightMap, float *heightmapBuffer)

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

	cudaGraphicsGLRegisterBuffer(&cuda_vb_resources[2], terrain.vbo[3], cudaGraphicsMapFlagsNone); // w_heightmap
	cudaGraphicsGLRegisterBuffer(&cuda_vb_resources[3], terrain.vbo[4], cudaGraphicsMapFlagsNone); // w_normals

	size_t s[4];
	s[0] = sizeof(GLfloat) * _size*_size;
	s[1] = sizeof(GLfloat) * _size*_size * 3;

	s[2] = sizeof(GLfloat) * _size*_size;
	s[3] = sizeof(GLfloat) * _size*_size * 3;


	cudaGraphicsMapResources(4, cuda_vb_resources, 0);

	cudaGraphicsResourceGetMappedPointer((void**)&d_heightmap, &s[0], cuda_vb_resources[0]);
	cudaGraphicsResourceGetMappedPointer((void**)&d_normals, &s[1], cuda_vb_resources[1]);

	cudaGraphicsResourceGetMappedPointer((void**)&d_watermap, &s[2], cuda_vb_resources[2]);
	cudaGraphicsResourceGetMappedPointer((void**)&d_waterNormals, &s[3], cuda_vb_resources[3]);

	cudaGraphicsUnmapResources(4, cuda_vb_resources, 0);

	cudaMalloc((void**)&d_heightBuffer, sizeof(float) * _size * _size);
	cudaMalloc((void**)&d_outflow, sizeof(float) * _size * _size * 4);
	cudaMalloc((void**)&d_sediment, sizeof(float) * _size * _size);
	cudaMalloc((void**)&d_velocity, sizeof(float) * _size * _size * 2);

	setHeight(0.0f, d_heightmap, _size*_size);
	setHeight(0.0f, d_watermap, _size*_size);
	setHeight(0.0f, d_sediment, _size*_size);

	cuda_initArray << <(N * 4 + M - 1) / M, M >> > (d_outflow, 0.0f, _size *_size * 4);
	cuda_initArray << <(N * 2 + M - 1) / M, M >> > (d_velocity, 0.0f, _size *_size * 2);

	fetchHeight();
	mapNormals();
}

CudaTools::~CudaTools()
{
	cudaGLUnregisterBufferObject(_terrain->vbo[1]);
	cudaGLUnregisterBufferObject(_terrain->vbo[2]);

	cudaFree(d_heightBuffer);
	cudaFree(d_outflow);
	cudaFree(d_sediment);
	cudaFree(d_velocity);
}

void CudaTools::setHeight(float height, float* arr, float size)
{
	cuda_initArray << <(N + M - 1) / M, M >> > (arr, height, size);

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
	cuda_MapNormals << < (N + M - 1) / M, M >> >(d_heightmap, d_normals, h, _size);
	cudaDeviceSynchronize();

	cuda_MapNormals << < (N + M - 1) / M, M >> >(d_heightmap, d_watermap, d_waterNormals, h, _size);
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
	cuda_initArray << <(N + M - 1) / M, M >> > (d_heightBuffer, 0, _size);
	cudaDeviceSynchronize();
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

void CudaTools::waterFlow(float H)
{
	waterFlowI << <(N + M - 1) / M, M >> >(d_heightmap, d_watermap, d_outflow, h, _size);
	cudaDeviceSynchronize();
	waterFlowII << <(N + M - 1) / M, M >> >(d_heightmap, d_normals, d_sediment, d_watermap, d_outflow, d_velocity, _size);
	cudaDeviceSynchronize();
	waterFlowIII << <(N + M - 1) / M, M >> >(d_sediment, d_heightBuffer, d_velocity, _size);
	cudaDeviceSynchronize();
	waterFlowIV << <(N + M - 1) / M, M >> > (d_sediment, d_heightBuffer, _size);

	cudaDeviceSynchronize();
}

void CudaTools::addWater(float factor)
{
	cuda_Add << <(N + M - 1) / M, M >> >(d_watermap, factor, _size);
}

void CudaTools::raindrop(const vec2 & position, float outerRadius, float innerRadius, float factor)
{
	int _x = floor(position.x - outerRadius);
	int _y = floor(position.y - outerRadius);

	int size = ceil(2 * outerRadius) + 1;

	int n = size*size;

	cuda_elevate << <(n + M - 1) / M, M >> >(d_watermap, _x, _y, position.x, position.y, outerRadius, innerRadius, factor, size, _size);
	cudaDeviceSynchronize();
}


