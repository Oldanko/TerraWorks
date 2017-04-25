#include "Tools.h"
#include "CudaTools.h"

void Tool::increaseInner()
{
	m_innerRadius += 0.1 * m_innerRadius;
	if (m_innerRadius > 1000.0f)
		m_innerRadius = 1000.0f;
	if (m_innerRadius > m_outerRadius)
		m_outerRadius = m_innerRadius;

}

void Tool::increaseOuter()
{
	m_outerRadius += 0.1 * m_outerRadius;
	if (m_outerRadius > 1000.0f)
		m_outerRadius = 1000.0f;
}

void Tool::decreaseInner()
{
	m_innerRadius -= 0.1 * m_innerRadius;
	if (m_innerRadius < 0.5)
		m_innerRadius = 0.5;
}

void Tool::decreaseOuter()
{

	m_outerRadius -= 0.1 * m_outerRadius;
	if (m_outerRadius < 0.5)
		m_outerRadius = 0.5;
	if (m_outerRadius < m_innerRadius)
		m_innerRadius = m_outerRadius;
}

float Tool::innerRadius()
{
	return m_innerRadius;
}

float Tool::outerRadius()
{
	return m_outerRadius;
}


void ToolElevate::apply(const vec3 &position)
{
	cudaTools->elevate(position.xy, m_outerRadius, m_innerRadius, m_factor);
}

void ToolAveragize::apply(const vec3 &position)
{
	cudaTools->averagize(position.xy, m_outerRadius, m_innerRadius, m_factor);
}


void ToolPlateau::apply(const vec3 &position)
{
	cudaTools->plateau(position, m_outerRadius, m_innerRadius, m_factor, m_above, m_below);
}
