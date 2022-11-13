#include "Renderer.h"
#include <fstream>

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	camera = new Camera();
	triangle = Mesh::GenerateQuad();

	texture0 = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	texture1 = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!texture0 || !texture1)
		return;

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess())
		return;

	filtering = true;
	repeating = false;
	
	init = true;
}

Renderer::~Renderer(void)
{
	delete triangle;
	delete shader;
	glDeleteTextures(1, &texture0);
	glDeleteTextures(1, &texture1);
}

void Renderer::UpdateScene(float dt)
{
	deltaTime = dt;
}

void Renderer::UpdateTextureMatrix(float value) 
{
	Matrix4 push = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4 pop = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4 rotation = Matrix4::Rotation(value, Vector3(0, 0, 1));

	textureMatrix = pop * rotation * push;
	//textureMatrix =  rotation * push;
}

void Renderer::ToggleRepeating() 
{
	repeating = !repeating;
	SetTextureRepeating(texture0, repeating);
	SetTextureRepeating(texture1, repeating);
}

void Renderer::ToggleFiltering() 
{
	//GL_LINEAR_MIPMAP_LINEAR = Trilinear Filtering
	//GL_LINEAR = Bilinear Filtering

	filtering = !filtering;
	glBindTexture(GL_TEXTURE_2D, texture0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::RenderScene() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	GLint u_Time = glGetUniformLocation(shader->GetProgram(), "time");
	glUniform1f(u_Time, deltaTime);

	GLint u_SinTime = glGetUniformLocation(shader->GetProgram(), "sinTime");
	glUniform1f(u_SinTime, sin(deltaTime));

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex0"), 0); //this last parameter
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex1"), 1); //this last parameter
	
	glActiveTexture(GL_TEXTURE0); // should match this number!
	glBindTexture(GL_TEXTURE_2D, texture0);
	
	glActiveTexture(GL_TEXTURE1); // should match this number!
	glBindTexture(GL_TEXTURE_2D, texture1);

	triangle->Draw();
}