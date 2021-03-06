#version 430 core

in vec3 Normal;
in vec2 Position;
in float Height;

out vec4 outColor;

layout(location = 2) uniform vec2 position;
layout(location = 3) uniform float outerRadius;
layout(location = 4) uniform float innerRadius;
vec3 sunVec = normalize(vec3(0, 1, 0));

void main()
{	

	float diff = max(dot(normalize(Normal), sunVec), 0.0);
	outColor = vec4(vec3(1.0)*diff*0.9, 1.0);
	if (abs((Position.x - position.x)*(Position.x - position.x) + (Position.y - position.y)*(Position.y - position.y) - outerRadius*outerRadius) < outerRadius/5)
		outColor = vec4(vec3(0.0f, 1.0, 0.0)*diff*0.9, 1.0);
	if(abs((Position.x - position.x)*(Position.x - position.x) + (Position.y - position.y)*(Position.y - position.y) - innerRadius*innerRadius) < innerRadius/5)
		outColor = vec4(vec3(1.0f, 0.0f, 0.0f)*diff*0.9f, 1.0f);

}