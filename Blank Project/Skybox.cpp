#include "Skybox.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/Shader.h"

Skybox::Skybox()
{
	quad = Mesh::GenerateQuad();
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	skyboxTex = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
}

Skybox::~Skybox()
{
	delete quad;
	delete skyboxShader;
	
	glDeleteTextures(1, &skyboxTex);
}

void Skybox::Draw()
{
	if(quad != nullptr)
		quad->Draw();
}