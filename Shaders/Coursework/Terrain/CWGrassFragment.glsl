#version 330 core

uniform sampler2D diffuseTex;

//Directional Light
uniform vec3 lightDir;
uniform vec4 lightDirColour;
uniform float lightDirIntensity;

//Point Light
uniform int numPointLights;
uniform vec3 pointLightPos[50];
uniform vec4 pointLightColour[50];
uniform float pointLightRadius[50];
uniform float pointLightIntensity[50];

//This should be same from Geometry Shader if there is any or else from Fragment Shader
in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

vec3 CalcDirLight(vec3 normal, vec4 diffuseFinal);
vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 normal, vec4 diffuseFinal);

void main(void)
{
	vec4 albedoColor = texture(diffuseTex, IN.texCoord);
	if(albedoColor.a < 0.4) discard;

	vec3 normal = IN.normal;

	vec3 result = vec3(0.0);
	result = CalcDirLight(normal, albedoColor);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLightColour[i], pointLightPos[i], pointLightRadius[i], pointLightIntensity[i], normal, albedoColor);
	}
	
	fragColour = vec4(result, 1.0);
}

vec3 CalcDirLight(vec3 normal, vec4 diffuseFinal)
{
	vec3 albedoColor = diffuseFinal.rgb;

	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDir);

	float NdotL = max(dot(N, L), 0.0);

	vec3 ambient = 0.3f * lightDirColour.rgb;
	vec3 diffuse = (NdotL * lightDirIntensity) * lightDirColour.rgb;

	return (ambient + diffuse) * albedoColor;
}

vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 normal, vec4 diffuseFinal)
{
	vec3 albedoColor = diffuseFinal.rgb;

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