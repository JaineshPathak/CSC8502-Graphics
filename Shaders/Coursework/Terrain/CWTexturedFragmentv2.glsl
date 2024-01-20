#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;

uniform vec3 cameraPos;
uniform bool hasBumpTex = true;

//Directional Light
uniform int flipLightDir;
uniform vec3 lightDir;
uniform vec4 lightDirColour;
uniform float lightDirIntensity;

//Point Light
uniform int numPointLights;
uniform vec3 pointLightPos[50];
uniform vec4 pointLightColour[50];
uniform float pointLightRadius[50];
uniform float pointLightIntensity[50];

uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform int enableFog;
uniform vec4 fogColour;

uniform float u_time;

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

vec3 CalcDirLight(vec3 viewDir, vec3 normal);
vec3 CalcPointLight(vec4 _pointLightColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _normal);

float ShadowCalc(float NdotL)
{
	vec3 projCoords = IN.fragPosLightSpace.xyz / IN.fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowTex, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.05 * (1.0 - NdotL), 0.005);
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

	float diffuseAlpha = texture(diffuseTex, IN.texCoord).a;
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	vec3 normal = IN.normal;
	if(hasBumpTex)
	{
		normal = texture(bumpTex, IN.texCoord).xyz;
		normal = normal * 2.0 - 1.0;
		normal.xy *= 1.0;
		normal = normalize(TBN * normalize(normal));
	}

	vec3 result = vec3(0.0);
	result += CalcDirLight(viewDir, normal);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLightColour[i], pointLightPos[i], pointLightRadius[i], pointLightIntensity[i], viewDir, normal);
	}

	fragColour = vec4(result, diffuseAlpha);
	if(enableFog == 1)
	{
		fragColour = mix(vec4(fogColour.xyz, diffuseAlpha), fragColour, IN.visibility);
	}
	//fragColour = vec4(1.0);
}

vec3 CalcDirLight(vec3 viewDir, vec3 normal)
{
	vec3 albedoColor = texture(diffuseTex, IN.texCoord).rgb;

	vec3 V = viewDir;
	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDir);
	vec3 H = normalize(V + L);

	float NdotL = max(dot(N, L), 0.0001f);
	float NdotH = dot(N, H);

	float specFactor = clamp(NdotH, 0.0, 1.0);
	specFactor = pow(specFactor, 32.0f);

	vec3 ambient = 0.1f * lightDirColour.rgb;
	vec3 diffuse = (NdotL * lightDirIntensity) * lightDirColour.rgb;
	vec3 specular = specFactor * lightDirColour.rgb;

	// calculate shadow
    float shadow = ShadowCalc(NdotL);

	return (ambient + (1.0 - shadow) * (diffuse + specular)) * albedoColor;

	/*vec3 incident = normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);	

	float lambert = max(dot(incident, normal), 0.0);
	float attenuation = 1.0f;

	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.1f * texture(diffuseTex, IN.texCoord).rgb;
	vec3 diffuseRGB = lightDirColour.rgb * texture(diffuseTex, IN.texCoord).rgb * lambert;
	vec3 specular = lightDirColour.rgb * (specularColour.rgb * specFactor);

	//--------------------
	//Shadow

	float shadow = 1.0;
	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w;
	if( abs(shadowNDC.x) < 1.0f && 
		abs(shadowNDC.y) < 1.0f &&
		abs(shadowNDC.z) < 1.0f)
	{
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex, biasCoord.xy).x;
		if(shadowZ < biasCoord.z)
			shadow = 0.0f;
	}

	//--------------------

	ambient *= shadow;
	diffuseRGB *= shadow;
	specular *= shadow;

	return (ambient + diffuseRGB + specular)*/
}

vec3 CalcPointLight(vec4 _pointLightColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _normal)
{
	vec3 albedoColor = texture(diffuseTex, IN.texCoord).rgb;

	vec3 V = _viewDir;
	vec3 N = normalize(_normal);
	vec3 L = normalize(_pointLightPos - IN.worldPos);
	vec3 H = normalize(V + L);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = dot(N, H);
	float Dist = length(_pointLightPos - IN.worldPos);
	float Atten = 1.0 - clamp((Dist / _pointLightRadius), 0.0, 1.0);
	
	float specFactor = clamp(NdotH, 0.0, 1.0);
	specFactor = pow(specFactor, 32.0f);

	vec3 ambient = 0.1f * _pointLightColour.rgb;
	vec3 diffuse = NdotL * _pointLightIntensity * _pointLightColour.rgb;
	vec3 specular = specFactor * _pointLightColour.rgb;

	ambient *= Atten;
	diffuse *= Atten;
	specular *= Atten;
	
	return (ambient + diffuse + specular) * albedoColor;

	/*vec3 incident = normalize(_pointLightPos - IN.worldPos);
	vec3 halfDir = normalize(incident + _viewDir);

	float lambert = max(dot(incident, _normal), 0.0);
	float distance = length(_pointLightPos - IN.worldPos);
	float attenuation = 1.0 - clamp( (distance / _pointLightRadius), 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, _normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.1f * texture(diffuseTex, IN.texCoord).rgb;
	vec3 diffuseRGB = _pointLightColour.rgb * texture(diffuseTex, IN.texCoord).rgb * lambert;
	vec3 specular = _pointLightColour.rgb * (_pointLightSpecularColour.rgb * specFactor);

	//--------------------
	//Shadow

	float shadow = 1.0;
	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w;
	if( abs(shadowNDC.x) < 1.0f && 
		abs(shadowNDC.y) < 1.0f &&
		abs(shadowNDC.z) < 1.0f)
	{
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex, biasCoord.xy).x;
		if(shadowZ < biasCoord.z)
			shadow = 0.0f;
	}

	//--------------------

	_pointLightIntensity = 0.0f;
	ambient *= attenuation * _pointLightIntensity;
	ambient *= shadow;

	diffuseRGB *= attenuation * _pointLightIntensity;
	diffuseRGB *= shadow;

	specular *= attenuation * _pointLightIntensity;
	specular *= shadow;

	return (ambient + diffuseRGB + specular);*/
}