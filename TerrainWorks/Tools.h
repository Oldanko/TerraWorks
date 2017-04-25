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
	float m_factorMax;
	float m_innerRadius;
	float m_outerRadius;
public:
	Tool(CudaTools * t);
	virtual void apply(const vec3& position) = 0;
	void increaseInner();
	void increaseOuter();
	void decreaseInner();
	void decreaseOuter();
	void increaseFactor();
	void decreaseFactor();

	void printFactor() const;

	float innerRadius() const;
	float outerRadius() const;
};

class ToolElevate : public Tool
{
public:
	ToolElevate(CudaTools *t) : Tool(t) { m_factor = 0.05f; m_factorMax = INFINITY; };
	void apply(const vec3&) override;
	void factorPositive();
	void factorNegative();
};

class ToolAveragize : public Tool
{
public:
	ToolAveragize(CudaTools *t) : Tool(t) { m_factor = 0.25f; m_factorMax = 0.5f;  };
	void apply(const vec3&) override;
};

class ToolPlateau : public Tool
{
	bool m_above;
	bool m_below;
public:
	ToolPlateau(CudaTools *t) : Tool(t) { m_factor = 0.005f; m_factorMax = 1.0f; };
	void apply(const vec3&) override;
};

