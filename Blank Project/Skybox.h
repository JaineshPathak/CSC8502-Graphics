#pragma once
#include <glad/glad.h>

class Shader;
class Mesh;

class Skybox
{
public:
	Skybox();
	~Skybox();
	
	Shader* GetSkyboxShader() { return skyboxShader; }
	void Draw();

protected:
	Mesh* quad;
	Shader* skyboxShader;
	GLuint skyboxTex;
};