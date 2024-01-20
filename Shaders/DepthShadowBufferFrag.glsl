#version 400 core

uniform sampler2D diffuseTex;

in vec2 TexCoords;

void main(void)
{
	//For Transparent textures like Trees
	float alpha = texture(diffuseTex, TexCoords).a;
	if(alpha < 0.5)
		discard;
}