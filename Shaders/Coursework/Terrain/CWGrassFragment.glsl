#version 430

const int MAX_POINT_LIGHTS = 100;

uniform sampler2D diffuseTex;

struct DirectionalLight
{
	vec4 lightDirection;
	vec4 lightColor;
};

struct PointLight
{
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightRadialIntensityData;
};

layout(std140, binding = 1) uniform u_DirectionLight
{
	DirectionalLight directionalLight;
};

layout(std140, binding = 2) uniform u_PointLights
{
	int numPointLights;
	PointLight pointLights[MAX_POINT_LIGHTS];
};

//This should be same from Geometry Shader if there is any or else from Fragment Shader
in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

vec3 CalcDirLight(vec3 normal, vec3 albedoColor);
vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 normal, vec3 albedoColor);

void main(void)
{
	vec4 albedoColor = texture(diffuseTex, IN.texCoord);
	if(albedoColor.a < 0.4) discard;

	vec3 normal = IN.normal;

	vec3 result = vec3(0.0);
	result = CalcDirLight(normal, albedoColor.rgb);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLights[i].lightColor, pointLights[i].lightPosition.xyz, pointLights[i].lightRadialIntensityData.x, pointLights[i].lightRadialIntensityData.y, normal, albedoColor.rgb);
	}
	
	fragColour = vec4(result, 1.0);
}

vec3 CalcDirLight(vec3 normal, vec3 albedoColor)
{
	vec3 lightDir = directionalLight.lightDirection.xyz;
	vec4 lightDirColour = directionalLight.lightColor;
	float lightDirIntensity = directionalLight.lightDirection.w;

	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDir);

	float NdotL = max(dot(N, L), 0.0);

	vec3 ambient = 0.3f * lightDirColour.rgb;
	vec3 diffuse = (NdotL * lightDirIntensity) * lightDirColour.rgb;

	return (ambient + diffuse) * albedoColor;
}

vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 normal, vec3 albedoColor)
{
	vec3 N = normalize(normal);
	vec3 L = normalize(pointLightPos - IN.worldPos);
	
	float NdotL = max(dot(N, L), 0.0);
	float Dist = length(pointLightPos - IN.worldPos);
	float Atten = 1.0 - clamp((Dist / pointLightRadius), 0.0, 1.0);
	
	vec3 ambient = 0.3f * pointLightColour.rgb;
	vec3 diffuse = NdotL * pointLightIntensity * pointLightColour.rgb;

	ambient *= Atten;
	diffuse *= Atten;
	
	return (ambient + diffuse) * albedoColor;
}