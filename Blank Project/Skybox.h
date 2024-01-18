#pragma once
#include <glad/glad.h>
#include <nclgl/Matrix4.h>
#include <memory>

class Shader;
class Skybox
{
public:
	Skybox();
	~Skybox();
	
	Shader* GetSkyboxShader() const { return m_SkyboxShader; }
	GLuint GetSkyboxCube() const { return m_SkyboxTex; }

	virtual void Draw();

private:
	Shader* m_SkyboxShader;
	GLuint m_SkyboxTex;
};