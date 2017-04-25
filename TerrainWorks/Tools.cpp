#include "Tools.h"
#include "CudaTools.h"

#include <iostream>

Tool::Tool(CudaTools * t)
{
	cudaTools = t;
	m_factor = 0.05f;
	m_innerRadius = 2.0f;
	m_outerRadius = 5.0f;
}

void Tool::increaseInner()
{
	m_innerRadius += 0.05 *m_innerRadius;
	if (m_innerRadius > 1000.0f)
		m_innerRadius = 1000.0f;
	if (m_innerRadius > m_outerRadius)
		m_outerRadius = m_innerRadius;

}

void Tool::increaseOuter()
{
	m_outerRadius += 0.05 *m_outerRadius;
	if (m_outerRadius > 1000.0f)
		m_outerRadius = 1000.0f;
}

void Tool::decreaseInner()
{
	m_innerRadius -= 0.05 * m_innerRadius;
	if (m_innerRadius < 0.5)
		m_innerRadius = 0.5;
}

void Tool::decreaseOuter()
{

	m_outerRadius -= 0.05 * m_outerRadius;
	if (m_outerRadius < 0.5)
		m_outerRadius = 0.5;
	if (m_outerRadius < m_innerRadius)
		m_innerRadius = m_outerRadius;
}

void Tool::increaseFactor()
{
	m_factor += 0.1 * m_factor;
	if (m_factor > m_factorMax)
		m_factor = m_factorMax;
}

void Tool::decreaseFactor()
{
	m_factor -= 0.1 * m_factor;
	if (m_factor < 0.0001f)
		m_factor = 0.0001f;
}

void Tool::printFactor() const
{
	std::cout << m_factor << "\n";
}



float Tool::innerRadius() const
{
	return m_innerRadius;
}

float Tool::outerRadius() const
{
	return m_outerRadius;
}


void ToolElevate::apply(const vec3 &position)
{
	cudaTools->elevate(position.xy, m_outerRadius, m_innerRadius, m_factor);
	cudaTools->mapNormals();
	cudaTools->fetchHeight();
}

void ToolElevate::factorPositive()
{
	if (m_factor < 0)
		m_factor = -m_factor;
}

void ToolElevate::factorNegative()
{
	if (m_factor > 0)
		m_factor = -m_factor;
}

void ToolAveragize::apply(const vec3 &position)
{
	cudaTools->averagize(position.xy, m_outerRadius, m_innerRadius, m_factor);
	cudaTools->mapNormals();
	cudaTools->fetchHeight();
}


void ToolPlateau::apply(const vec3 &position)
{
	cudaTools->plateau(position, m_outerRadius, m_innerRadius, m_factor, m_above, m_below);
	cudaTools->mapNormals();
	cudaTools->fetchHeight();
}
