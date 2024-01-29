#version 430

const int MAX_POINT_LIGHTS = 100;

uniform sampler2D diffuseSplatmapTex;		//0
uniform sampler2D diffuseGrassTex;			//1
uniform sampler2D bumpGrassTex;				//2
uniform sampler2D diffuseRocksTex;			//3
uniform sampler2D bumpRocksTex;				//4
uniform sampler2D diffuseGroundTex;			//5
uniform sampler2D bumpGroundTex;			//6
uniform sampler2D shadowTex;				//7

uniform vec3 cameraPos;
uniform bool hasBumpTex = true;
uniform float u_time;

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

struct EnvironmentData
{
	vec4 fogData;
	vec4 fogColor;
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

layout(std140, binding = 3) uniform u_EnvironmentData
{
	EnvironmentData envData;
};

in Vertex
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
} IN;

out vec4 fragColour;

vec3 CalcDirLight(vec3 viewDir, vec3 normal, vec3 albedoColor);
vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 viewDir, vec3 normal, vec3 albedoColor);

float ShadowCalc(float NdotL)
{
	vec3 projCoords = IN.fragPosLightSpace.xyz / IN.fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowTex, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - NdotL), 0.005);
	//float bias = 0.005;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowTex, 0);
	const int halfkernelWidth = 3;
	for(int x = -halfkernelWidth; x <= halfkernelWidth; x++)
	{
		for(int y = -halfkernelWidth; y <= halfkernelWidth; y++)
		{
			float pcfDepth = texture(shadowTex, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 0.7 : 0.0;
		}
	}
	
	//shadow /= 9.0;
	shadow /= ((halfkernelWidth * 2.0 + 1.0) * (halfkernelWidth * 2.0 + 1.0));

	if(projCoords.z > 1.0)
        shadow = 0.0;
	
	return shadow;
}

void main(void)
{
	//==============================================================================================

	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec4 splatmap = texture(diffuseSplatmapTex, IN.texCoord / 16.0f);

	float grassAmount = splatmap.r;
	float rocksAmount = splatmap.g;
	float groundAmount = splatmap.b;

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	vec3 normalFinal = IN.normal;
	if(hasBumpTex)
	{
		vec3 grassNormal = texture(bumpGrassTex, IN.texCoord).xyz * grassAmount;
		vec3 rocksNormal = texture(bumpRocksTex, IN.texCoord).xyz * rocksAmount;
		vec3 groundNormal = texture(bumpGroundTex, IN.texCoord).xyz * groundAmount;
		
		normalFinal = grassNormal + rocksNormal + groundNormal;
		normalFinal = normalFinal * 2.0 - 1.0;
		normalFinal.xy *= 1.0;
		normalFinal = normalize(TBN * normalize(normalFinal));
	}

	vec4 grassTex = texture(diffuseGrassTex, IN.texCoord) * grassAmount;
	vec4 rocksTex = texture(diffuseRocksTex, IN.texCoord) * rocksAmount;
	vec4 groundTex = texture(diffuseGroundTex, IN.texCoord) * groundAmount;

	vec4 diffuseFinal = grassTex + rocksTex + groundTex;

	vec3 result = vec3(0.0);
	result = CalcDirLight(viewDir, normalFinal, diffuseFinal.rgb);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLights[i].lightColor, pointLights[i].lightPosition.xyz, pointLights[i].lightRadialIntensityData.x, pointLights[i].lightRadialIntensityData.y, viewDir, normalFinal, diffuseFinal.rgb);
	}

	fragColour = vec4(result, 1.0);

	bool fogEnabled = bool(envData.fogData.x);
	if(fogEnabled)
	{
		fragColour = mix(vec4(envData.fogColor.xyz, 1.0f), fragColour, IN.visibility);
	}
	//fragColour = vec4(1.0);
}

vec3 CalcDirLight(vec3 viewDir, vec3 normal, vec3 albedoColor)
{
	vec3 lightDir = directionalLight.lightDirection.xyz;
	vec4 lightDirColour = directionalLight.lightColor;
	float lightDirIntensity = directionalLight.lightDirection.w;

	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDir);
	
	float NdotL = max(dot(N, L), 0.0001f);
	
	vec3 ambient = 0.1f * lightDirColour.rgb;
	vec3 diffuse = (NdotL * lightDirIntensity) * lightDirColour.rgb;
	
	// calculate shadow
    float shadow = ShadowCalc(NdotL);

	return (ambient + (1.0 - shadow) * (diffuse)) * albedoColor;
}

vec3 CalcPointLight(vec4 pointLightColour, vec3 pointLightPos, float pointLightRadius, float pointLightIntensity, vec3 viewDir, vec3 normal, vec3 albedoColor)
{
	vec3 V = viewDir;
	vec3 N = normalize(normal);
	vec3 L = normalize(pointLightPos - IN.worldPos);
	vec3 H = normalize(V + L);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = dot(N, H);
	float Dist = length(pointLightPos - IN.worldPos);
	float Atten = 1.0 - clamp((Dist / pointLightRadius), 0.0, 1.0);
	
	float specFactor = clamp(NdotH, 0.0, 1.0);
	specFactor = pow(specFactor, 32.0f);

	vec3 ambient = 0.1f * pointLightColour.rgb;
	vec3 diffuse = NdotL * pointLightIntensity * pointLightColour.rgb;
	vec3 specular = specFactor * pointLightColour.rgb;

	ambient *= Atten;
	diffuse *= Atten;
	specular *= Atten;
	
	return (ambient + diffuse + specular) * albedoColor;
}