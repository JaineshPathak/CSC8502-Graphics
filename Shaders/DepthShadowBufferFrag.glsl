#version 400 core

uniform sampler2D diffuseTex;

in vec2 TexCoords;

void main(void)
{
	//For Transparent textures like Trees
	vec4 diffuseSample = texture(diffuseTex, TexCoords);
	if(diffuseSample.a == 0.0)
		discard;
}