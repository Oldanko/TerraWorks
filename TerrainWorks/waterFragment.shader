#version 430 core

in vec3 Normal;
in vec2 Position;
in float Height;

out vec4 outColor;

vec3 sunVec = normalize(vec3(0, 1, 0));

void main()
{	

	float diff = max(dot(normalize(Normal), sunVec), 0.0);
	outColor = vec4(vec3(0.0, 0.0, 1.0)*diff, 1.0f);
}