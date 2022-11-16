#version 330 core

uniform sampler2D bumpTex;
uniform sampler2D diffuseSplatmapTex;
uniform sampler2D diffuseGrassTex;
uniform sampler2D diffuseRocksTex;
uniform sampler2D diffuseGroundTex;

uniform vec3 cameraPos;

//Directional Light
uniform vec3 lightDir;
uniform vec4 lightDirColour;

//Point Light
uniform int numPointLights;

uniform vec3 pointLightPos[50];
uniform vec4 pointLightColour[50];
uniform vec4 pointLightSpecularColour[50];
uniform float pointLightRadius[50];

uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform float u_time;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal, vec4 diffuseFinal);
vec3 CalcPointLight(vec4 _pointLightColour, vec4 _pointLightSpecularColour, vec3 _pointLightPos, float _pointLightRadius, vec3 _viewDir, vec3 _bumpNormal, vec4 _diffuseFinal);

void main(void)
{
	//==============================================================================================

	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 bumpNormal = texture(bumpTex, IN.texCoord ).xyz;
	bumpNormal = bumpNormal * 2.0 - 1.0;
	bumpNormal.xy *= 1.0;
	bumpNormal = normalize(bumpNormal);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	bumpNormal = normalize(TBN * bumpNormal);



	float grassAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).r;
	float rocksAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).g;
	float groundAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).b;

	vec4 grassTex = texture(diffuseGrassTex, IN.texCoord) * grassAmount;
	vec4 rocksTex = texture(diffuseRocksTex, IN.texCoord) * rocksAmount;
	vec4 groundTex = texture(diffuseGroundTex, IN.texCoord) * groundAmount;

	vec4 splatmap = texture(diffuseSplatmapTex, IN.texCoord / 16.0f);

	vec4 diffuseFinal = grassTex + rocksTex + groundTex;



	vec3 result = vec3(0.0);
	result += CalcDirLight(viewDir, bumpNormal, diffuseFinal);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLightColour[i], pointLightSpecularColour[i], pointLightPos[i], pointLightRadius[i], viewDir, bumpNormal, diffuseFinal);
	}

	fragColour = vec4(result, 1.0);
	//fragColour = vec4(1.0);
}

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal, vec4 diffuseFinal)
{
	vec3 incident = normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);	

	float lambert = max(dot(incident, IN.normal), 0.0);
	float attenuation = 1.0f;

	float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.3f * diffuseFinal.rgb;
	vec3 diffuseRGB = lightDirColour.rgb * diffuseFinal.rgb * lambert;
	vec3 specular = lightDirColour.rgb * (specularColour.rgb * specFactor);

	return (ambient + diffuseRGB);
}

vec3 CalcPointLight(vec4 _pointLightColour, vec4 _pointLightSpecularColour, vec3 _pointLightPos, float _pointLightRadius, vec3 _viewDir, vec3 _bumpNormal, vec4 _diffuseFinal)
{
	vec3 incident = normalize(_pointLightPos - IN.worldPos);
	vec3 halfDir = normalize(incident + _viewDir);

	/*mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	_bumpNormal = normalize(TBN * _bumpNormal);*/

	float lambert = max(dot(incident, IN.normal), 0.0);
	float distance = length(_pointLightPos - IN.worldPos);
	float attenuation = 1.0 - clamp( (distance / _pointLightRadius), 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.1f * _diffuseFinal.rgb;
	vec3 diffuseRGB = _pointLightColour.rgb * _diffuseFinal.rgb * lambert;
	vec3 specular = _pointLightColour.rgb * (_pointLightSpecularColour.rgb * specFactor);

	ambient *= attenuation;
	diffuseRGB *= attenuation;
	specular *= attenuation;

	return (ambient + diffuseRGB);
}