#include "Renderer.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)
{
	cameraMain = new Camera();
	cameraMain->SetDefaultSpeed(250.0f);

	terrainMain = new TerrainHeightmap(TEXTUREDIRCOURSETERRAIN"Terrain_heightmap.png", 32.0f, 32.0f, 16.0f, 16.0f);
	if (!terrainMain->InitSuccess())
	{
		std::cout << "Renderer - Something went wrong loading Terrain Main!\n";
		return;
	}
	Vector3 terrainHeightmapSize = terrainMain->GetHeightMapSize();
	cameraMain->SetPosition(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f));

	projMatrix = Matrix4::Perspective(1.0, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer(void)	
{
	delete cameraMain;
	delete terrainMain;
}

void Renderer::UpdateScene(float dt) 
{
	cameraMain->UpdateCamera(dt);
	viewMatrix = cameraMain->BuildViewMatrix();
}

bool Renderer::BindTexture(GLuint texID, GLuint unit, const std::string& uniformName, Shader* s)
{
	GLint uniformID = glGetUniformLocation(s->GetProgram(), uniformName.c_str());

	if (uniformID < 0)
	{
		std::cout << "Trying to bind invalid 2D texture uniform!\n"; //Put breakpoint on this!
		return false;
	}

	if (currentShader != s) 
	{
		std::cout << "Trying to set shader uniform on wrong shader!\n";
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + unit); //A neat trick!
	glBindTexture(GL_TEXTURE_2D, texID);

	glUniform1i(uniformID, unit);

	return true;
}

void Renderer::RenderScene()	
{
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawMainTerrain();
}

void Renderer::DrawMainTerrain()
{
	BindShader(terrainMain->GetTerrainShader());
	UpdateShaderMatrices();

	/*glUniform1i(glGetUniformLocation(terrainMain->GetTerrainShader()->GetProgram(), "diffuseGrassTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainMain->GetTerrainTextureGrass());*/

	BindTexture(terrainMain->GetTerrainTextureSplatmap(), 0, "diffuseSplatmapTex", terrainMain->GetTerrainShader());
	BindTexture(terrainMain->GetTerrainTextureGrass(), 1, "diffuseGrassTex", terrainMain->GetTerrainShader());
	BindTexture(terrainMain->GetTerrainTextureRocks(), 2, "diffuseRocksTex", terrainMain->GetTerrainShader());
	BindTexture(terrainMain->GetTerrainTextureGround(), 3, "diffuseGroundTex", terrainMain->GetTerrainShader());

	terrainMain->Draw();
}