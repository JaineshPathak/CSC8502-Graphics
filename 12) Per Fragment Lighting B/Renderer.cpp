#include "..\15) Deferred Shading\Renderer.h"
#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include <string>

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	timer = parent.GetTimer();

	heightMap = new HeightMap(TEXTUREDIR"noise.png");

	terrainTexture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	bumpmap = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//shader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	shader = new Shader("BumpVertex.glsl", "BumpFragment.glsl");

	if (!shader->LoadSuccess() || !terrainTexture || !bumpmap)
		return;

	SetTextureRepeating(terrainTexture, true);
	SetTextureRepeating(bumpmap, true);

	Vector3 heightMapSize = heightMap->GetHeightMapSize();
	camera = new Camera(-45.0f, 0.0f, 0.0f, heightMapSize * Vector3(0.5f, 5.0f, 0.5f));
	camera->SetDefaultSpeed(120.0f);

	//light = new Light(heightMapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(), Vector4(1.0, 0.5, 0, 1), heightMapSize.x * 0.5f);
	Vector3 lightCornerPos[4] =
	{
		Vector3(0, 1.5f, 0),
		Vector3(0, 1.5f, 1),
		Vector3(1, 1.5f, 0),
		Vector3(1, 1.5f, 1)
	};

	Vector4 lightColors[4]
	{
		Vector4(),
		Vector4(1, 0, 0, 1),
		Vector4(0, 1, 0, 1),
		Vector4(0, 0, 1, 1)
	};

	//for (int i = 0; i < 4; i++)
		//light[i] = new Light(heightMapSize * lightCornerPos[i], lightColors[i], Vector4(1, 1, 1, 1), heightMapSize.x * 0.5f);

	light = new Light(heightMapSize * Vector3(0.5f, 1.5, 0.5f), Vector4(), heightMapSize.x * 0.5f);

	projMatrix = Matrix4::Perspective(0.01f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	init = true;
}

Renderer::~Renderer(void)
{
	delete camera;
	delete heightMap;
	delete shader;

	//for (int i = 0; i < 4; i++)
		//delete light[i];

	delete light;
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpmap);

	glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&camera->getPosition());

	glUniform1f(glGetUniformLocation(shader->GetProgram(), "u_time"), timer->GetTotalTimeSeconds());

	UpdateShaderMatrices();

	SetShaderLight(*light);		//Single Light

	//4 Lights in 4 corners
	/*for (size_t i = 0; i < 4; i++)
	{
		std::string lightPosName = "lightPos[" + std::to_string(i) + "]";
		std::string lightColorName = "lightColour[" + std::to_string(i) + "]";
		std::string specularColourName = "specularColour[" + std::to_string(i) + "]";
		std::string lightRadiusName = "lightRadius[" + std::to_string(i) + "]";

		glUniform3fv(glGetUniformLocation(shader->GetProgram(), lightPosName.c_str()), 1, (float*)&light[i]->GetPosition());
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), lightColorName.c_str()), 1, (float*)&light[i]->GetColour());
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), specularColourName.c_str()), 1, (float*)&light[i]->GetSpecularColour());
		glUniform1f(glGetUniformLocation(shader->GetProgram(), lightRadiusName.c_str()), light[i]->GetRadius());
	}*/

	heightMap->Draw();
}