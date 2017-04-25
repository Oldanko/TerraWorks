#pragma once
#define GLM_SWIZZLE

#include <glm\glm.hpp>

using namespace glm;

class CudaTools;

class Tool
{
protected:
	CudaTools * cudaTools;
	float m_factor;
	float m_innerRadius;
	float m_outerRadius;
public:
	virtual void apply(const vec3& position) = 0;
	void increaseInner();
	void increaseOuter();
	void decreaseInner();
	void decreaseOuter();

	float innerRadius();
	float outerRadius();
};

class ToolElevate : public Tool
{
public:
	void apply(const vec3&) override;
};

class ToolAveragize : public Tool
{
public:
	void apply(const vec3&) override;
};

class ToolPlateau : public Tool
{
	bool m_above;
	bool m_below;
public:
	void apply(const vec3&) override;
};

