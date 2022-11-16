#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "TerrainNode.h"
#include "RockNode.h"
#include "TreePropNode.h"
#include "FileHandler.h"
#include "Skybox.h"

#include "../Third Party/imgui/imgui.h"
#include "../Third Party/imgui/imgui_impl_opengl3.h"
#include "../Third Party/imgui/imgui_impl_win32.h"

class Camera;
class Light;
class DirectionalLight;

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
	bool blendFix = true;
	Camera* cameraMain;

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
	MeshMaterial* rockMaterial;
	
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

	void NewTreeProp(Mesh* m, MeshMaterial* mMat, SceneNode* parent, bool isTransparent = false);
	void NewTreeProp(Mesh* m, MeshMaterial* mMat, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent, bool isTransparent = false);
	void LoadTreeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, SceneNode* parent, bool isTransparent = false);
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Castle Main
	SceneNode* castleParentNode;
	Mesh* castleMesh;
	MeshMaterial* castleMaterial;
	TreePropNode* castlePropNode;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Castle Pillar
	SceneNode* castlePillarParentNode;
	Mesh* castlePillarMesh;
	MeshMaterial* castlePillarMaterial;
	TreePropNode* castlePillarPropNode;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Castle Pillar
	SceneNode* castleBridgeParentNode;
	Mesh* castleBridgeMesh;
	MeshMaterial* castleBridgeMaterial;
	TreePropNode* castleBridgePropNode;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Ruins Main
	SceneNode* ruinsParentNode;
	Mesh* ruinsMesh;
	MeshMaterial* ruinsMaterial;
	TreePropNode* ruinsPropNode;
	//--------------------------------------------------------------------

	//Lights
	DirectionalLight* dirLight;

	//--------------------------------------------------------------------

	//Cubemap
	Skybox* skybox;
	void DrawSkybox();

	//Timer
	GameTimer* timer;

	//void DrawMainTerrain();
	void DrawNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNode(SceneNode* n, bool includingChild = true);

	std::vector<SceneNode*> transparentNodesList;
	std::vector<SceneNode*> opaqueNodesList;
};