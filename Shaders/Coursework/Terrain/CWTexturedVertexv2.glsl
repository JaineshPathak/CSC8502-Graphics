#version 430 core

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

uniform mat4 modelMatrix;
uniform mat4 shadowMatrix;
uniform mat4 lightSpaceMatrix;
uniform int enableFog;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
	vec4 fragPosLightSpace;

	float visibility;
} OUT;

const float density = 0.00015f;
const float gradient = 1.5f;

void main(void)
{
	OUT.colour = colour;
	OUT.texCoord = texCoord;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	vec4 worldPos = (modelMatrix * vec4(position, 1));

	vec4 posRelativeToCam = viewMatrix * worldPos;
	OUT.worldPos = worldPos.xyz;

	gl_Position = (projMatrix * viewMatrix) * worldPos;

	if(enableFog == 1)
	{
		float distance = length(posRelativeToCam.xyz);
		OUT.visibility = exp(-pow((distance * density), gradient));
		OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
	}

	OUT.fragPosLightSpace = lightSpaceMatrix * vec4(OUT.worldPos, 1.0);
}