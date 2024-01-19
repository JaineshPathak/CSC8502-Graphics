#version 330 core

uniform sampler2D diffuseTex;

in vec2 TexCoords;

out vec4 fragColor;

const float near_plane = 1.0;
const float far_plane = 15000.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main(void)
{
	float depthValue = texture(diffuseTex, TexCoords).r;
	//fragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0);
	fragColor = vec4(vec3(depthValue), 1.0);
	//fragColor = vec4(1.0, 0.0f, 0.0, 1.0);
}