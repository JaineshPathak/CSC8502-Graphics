#version 330 core

uniform bool hasBumpTex = true;
uniform sampler2D bumpTex;
uniform sampler2D diffuseSplatmapTex;
uniform sampler2D diffuseGrassTex;
uniform sampler2D diffuseRocksTex;
uniform sampler2D diffuseGroundTex;
uniform sampler2D shadowTex;

uniform vec3 cameraPos;

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

uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform int enableFog;
uniform vec4 fogColour;

uniform float u_time;
const float PI = 3.14159265359;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;

	float visibility;
} IN;

out vec4 fragColour;

vec3 CalcDirLight(vec3 viewDir, vec3 normal, vec4 diffuseFinal);
vec3 CalcPointLight(vec4 _pointLightColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _normal, vec4 _diffuseFinal);

void main(void)
{
	//==============================================================================================

	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	vec3 normal = IN.normal;
	if(hasBumpTex)
	{
		normal = texture(bumpTex, IN.texCoord ).xyz;
		normal = normal * 2.0 - 1.0;
		normal.xy *= 1.0;
		normal = normalize(TBN * normalize(normal));
	}

	vec4 splatmap = texture(diffuseSplatmapTex, IN.texCoord / 16.0f);

	float grassAmount = splatmap.r;
	float rocksAmount = splatmap.g;
	float groundAmount = splatmap.b;

	vec4 grassTex = texture(diffuseGrassTex, IN.texCoord) * grassAmount;
	vec4 rocksTex = texture(diffuseRocksTex, IN.texCoord) * rocksAmount;
	vec4 groundTex = texture(diffuseGroundTex, IN.texCoord) * groundAmount;

	vec4 diffuseFinal = grassTex + rocksTex + groundTex;

	vec3 result = vec3(0.0);
	result = CalcDirLight(viewDir, normal, diffuseFinal);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLightColour[i], pointLightPos[i], pointLightRadius[i], pointLightIntensity[i], viewDir, normal, diffuseFinal);
	}

	fragColour = vec4(result, 1.0);
	if(enableFog == 1)
	{
		fragColour = mix(vec4(fogColour.xyz, 1.0f), fragColour, IN.visibility);
	}
	//fragColour = vec4(1.0);
}

vec3 CalcDirLight(vec3 viewDir, vec3 normal, vec4 diffuseFinal)
{
	vec3 albedoColor = diffuseFinal.rgb;

	vec3 V = viewDir;
	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDir);
	vec3 H = normalize(V + L);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = dot(N, H);

	float specFactor = clamp(NdotH, 0.0, 1.0);
	//specFactor = pow(specFactor, 1.0f);	

	vec3 ambient = 0.1f * lightDirColour.rgb;
	vec3 diffuse = NdotL * lightDirIntensity * lightDirColour.rgb;
	vec3 specular = specFactor * lightDirColour.rgb;

	return (ambient + diffuse) * albedoColor;

	/*vec3 incident = normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);	

	float lambert = max(dot(incident, IN.normal), 0.0);
	float attenuation = 1.0f;

	float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor = pow(specFactor, 800.0f);

	vec3 ambient = 0.3f * diffuseFinal.rgb;
	vec3 diffuseRGB = lightDirColour.rgb * diffuseFinal.rgb * lambert;
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

	return (ambient + diffuseRGB + specular) * lightDirIntensity;*/
}

vec3 CalcPointLight(vec4 _pointLightColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _normal, vec4 _diffuseFinal)
{
	vec3 albedoColor = _diffuseFinal.rgb;

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

	float lambert = max(dot(incident, IN.normal), 0.0);
	float distance = length(_pointLightPos - IN.worldPos);
	float attenuation = 1.0 - clamp( (distance / _pointLightRadius), 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor = pow(specFactor, 800.0f);

	vec3 ambient = 0.1f * _diffuseFinal.rgb;
	vec3 diffuseRGB = _pointLightColour.rgb * _diffuseFinal.rgb * lambert;
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

	ambient *= attenuation * _pointLightIntensity;
	ambient*= shadow;

	diffuseRGB *= attenuation * _pointLightIntensity;
	diffuseRGB *= shadow;
	
	specular *= attenuation * _pointLightIntensity;
	specular *= shadow;

	return (ambient + diffuseRGB + specular);*/
}