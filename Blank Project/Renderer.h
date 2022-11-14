#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "TerrainNode.h"
#include "RockNode.h"

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
	SceneNode* rootNode;
	TerrainNode* terrainNode;

	SceneNode* rocksParentNode;
	RockNode* rockNode;

	SceneNode* treesParentNode;

	Camera* cameraMain;

	void DrawMainTerrain();
	void DrawNode(SceneNode* n);
	void DrawRocks();
	TerrainHeightmap* terrainMain;

	//Testing
};