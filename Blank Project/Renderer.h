#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "TerrainHeightmap.h"

class Camera;

class Renderer : public OGLRenderer	
{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;

	 bool BindTexture(GLuint texID, GLuint unit, const std::string& uniformName, Shader* s);

protected:
	Camera* cameraMain;


	void DrawMainTerrain();
	TerrainHeightmap* terrainMain;
};
