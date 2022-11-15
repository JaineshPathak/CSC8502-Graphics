#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "TerrainNode.h"
#include "RockNode.h"
#include "TreePropNode.h"
#include "FileHandler.h"

#include "../Third Party/imgui/imgui.h"
#include "../Third Party/imgui/imgui_impl_opengl3.h"
#include "../Third Party/imgui/imgui_impl_win32.h"

class Camera;

class Renderer : public OGLRenderer	
{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;
	 void UpdateImGui();

	 //bool BindTexture(GLuint texID, GLuint unit, const std::string& uniformName, Shader* s);

protected:
	SceneNode* rootNode;
	TerrainNode* terrainNode;
	//TerrainHeightmap* terrainMain;
	Vector3 terrainHeightmapSize;

	//--------------------------------------------------------------------
	//Rocks
	Shader* basicDiffuseShader;
	SceneNode* rocks2ParentNode;
	SceneNode* rocks5aParentNode;
	GLuint rockTexture;
	
	Mesh* rock2Mesh;
	Mesh* rock5aMesh;

	void NewRock(Mesh* m, GLuint t, SceneNode* parent);
	void NewRock(Mesh* m, GLuint t, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent);
	void LoadRockData(const std::string& fileName, Mesh* m, GLuint t, SceneNode* parent);
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Trees
	SceneNode* treesParentNode;
	Mesh* treeMesh;
	MeshMaterial* treeMaterial;
	TreePropNode* treePropNode;

	void NewTreeProp(Mesh* m, MeshMaterial* mMat, SceneNode* parent);
	void NewTreeProp(Mesh* m, MeshMaterial* mMat, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent);
	void LoadTreeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, SceneNode* parent);
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Castle Main
	SceneNode* castleParentNode;
	Mesh* castleMesh;
	MeshMaterial* castleMaterial;
	TreePropNode* castlePropNode;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Ruins Main
	SceneNode* ruinsParentNode;
	Mesh* ruinsMesh;
	MeshMaterial* ruinsMaterial;
	TreePropNode* ruinsPropNode;
	//--------------------------------------------------------------------

	Camera* cameraMain;

	//void DrawMainTerrain();
	void DrawNode(SceneNode* n);
	//void DrawRocks2();

	//Testing
};