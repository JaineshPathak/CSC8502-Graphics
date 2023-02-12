#pragma once
#include <glad/glad.h>

class Shader;

class Skybox
{
public:
	Skybox();
	~Skybox();
	
	Shader* GetSkyboxShader() { return skyboxShader; }
	GLuint GetSkyboxCube() { return skyboxTex; }

protected:
	Shader* skyboxShader;
	GLuint skyboxTex;
};