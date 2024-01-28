#version 330 core

layout(location = 0) in vec3 position;
layout(location = 3) in vec3 normal;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out Vertex
{
	vec3 normal;
} OUT;

void main (void)
{
	mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * modelMatrix)));
    OUT.normal = vec3(vec4(normalMatrix * normal, 0.0));
	
	gl_Position = viewMatrix * modelMatrix * vec4(position, 1.0);
}