#version 330 core

uniform sampler2D diffuseTex;
uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void)
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);


	float lambert = max(dot(incident, IN.normal), 0.0);
	float distance = length(lightPos - IN.worldPos);
	float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);
	//attenuation *= (sin(u_time * 2.0) + 1.0) / 2.0;

	float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor = pow(specFactor, 100.0f);

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb += surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * (specularColour.rgb * specFactor)) * attenuation;
	fragColour.rgb += surface * 0.1f;			//Ambient
	fragColour.a = diffuse.a;					//Copies Alpha
}