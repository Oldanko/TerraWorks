#version 430 core

layout(location = 0) in vec2 position;
layout(location = 1) in float height;
layout(location = 2) in float water;
layout(location = 3) in vec3 normal;

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform float H;

out vec3 Normal;
out vec2 Position;
out float Height;

void main()
{
	gl_Position = MVP * vec4(position.x, height*H + water, position.y, 1);
	Normal = vec3(normal.x*(H + 1), normal.y, normal.z*(H + 1)); //vec3(1.0);//normal;
	Height = height;
	Position = position;
}