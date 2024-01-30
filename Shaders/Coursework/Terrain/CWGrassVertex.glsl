#version 330 core

/*uniform mat4 viewMatrix;
uniform mat4 projMatrix;*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 9) in mat4 instanceModelMatrix;

out Vertex
{
	vec3 position;
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
	mat4 instanceModelMat;
} OUT;

void main (void)
{
	OUT.position = position;
	OUT.colour = colour;
	OUT.texCoord = texCoord;
	OUT.worldPos = (instanceModelMatrix * vec4(position, 1.0)).xyz;
	OUT.instanceModelMat = instanceModelMatrix;

	mat3 normalMatrix = transpose(inverse(mat3(instanceModelMatrix)));
	OUT.normal = normalize(normalMatrix * normalize(normal));

	//gl_Position = projMatrix * viewMatrix * instanceMatrix * vec4(position, 1.0);
	gl_Position = vec4(position, 1.0);
}