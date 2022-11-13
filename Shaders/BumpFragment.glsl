#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

/*uniform vec4 lightColour[4];
uniform vec4 specularColour[4];
uniform vec3 lightPos[4];
uniform float lightRadius[4];*/

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

void main(void)
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 bumpNormal = texture(bumpTex, IN.texCoord ).xyz;
	bumpNormal = bumpNormal * 2.0 - 1.0;
	bumpNormal.xy *= 0.8;
	bumpNormal = normalize(bumpNormal);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	/*for(int i = 0; i < 4; i++)
	{
		vec3 incident = normalize(lightPos[i] - IN.worldPos);
		vec3 halfDir = normalize(incident + viewDir);


		float lambert = max(dot(incident, IN.normal), 0.0);
		float distance = length(lightPos[i] - IN.worldPos);
		float attenuation = 1.0 - clamp(distance / lightRadius[i], 0.0, 1.0);
		//attenuation *= (sin(u_time * 2.0) + 1.0) / 2.0;

		float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
		specFactor = pow(specFactor, 10.0);

		vec3 surface += (diffuse.rgb * lightColour[i].rgb);
		fragColour.rgb += surface * lambert * attenuation;
		fragColour.rgb += (lightColour[i].rgb * (specularColour[i].rgb * specFactor)) * attenuation;
		fragColour.rgb += surface * 0.1f;			//Ambient
	}*/

	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	bumpNormal = normalize(TBN * bumpNormal);


	float lambert = max(dot(incident, bumpNormal), 0.0);
	float distance = length(lightPos - IN.worldPos);
	float attenuation = 1.0 - clamp( (distance / lightRadius), 0.0, 1.0);
	//attenuation *= (sin(u_time * 2.0) + 1.0) / 2.0;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * (specularColour.rgb * specFactor)) * attenuation;
	fragColour.rgb += surface * 0.1f;			//Ambient
	fragColour.a = diffuse.a;					//Copies Alpha
}