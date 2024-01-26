#version 330 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 9) in mat4 instanceMatrix;

out Vertex
{
	vec4 colour;
} OUT;

void main (void)
{
	OUT.colour = colour;

	gl_Position = projMatrix * viewMatrix * instanceMatrix * vec4(position, 1.0);
	//gl_Position = vec4(position, 1.0);
}