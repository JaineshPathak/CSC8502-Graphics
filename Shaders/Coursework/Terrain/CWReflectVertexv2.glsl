#version 430

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

struct EnvironmentData
{
	vec4 fogData;
	vec4 fogColor;
};

layout(std140, binding = 3) uniform u_EnvironmentData
{
	EnvironmentData envData;
};

uniform mat4 modelMatrix;
uniform mat4 textureMatrix;
uniform mat4 lightSpaceMatrix;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec2 texCoord;
in vec4 tangent;

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

void main(void)
{
	OUT.colour = colour;
	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
	
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	vec4 worldPos = (modelMatrix * vec4(position, 1));
	OUT.worldPos = worldPos.xyz;

	OUT.normal = normalize(normalMatrix * normalize(normal));

	gl_Position = (projMatrix * viewMatrix) * worldPos;

	vec4 posRelativeToCam = viewMatrix * worldPos;
	
	bool fogEnabled = bool(envData.fogData.x);
	if(fogEnabled)
	{
		float fogDensity = envData.fogData.y;
		float fogGradient = envData.fogData.z;

		float distance = length(posRelativeToCam.xyz);
		OUT.visibility = exp(-pow((distance * fogDensity), fogGradient));
		OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
	}

	OUT.fragPosLightSpace = lightSpaceMatrix * vec4(OUT.worldPos, 1.0);
}