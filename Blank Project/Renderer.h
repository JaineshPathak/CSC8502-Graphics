#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "TerrainNode.h"
#include "RockNode.h"
#include "TreePropNode.h"
#include "AnimMeshNode.h"
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

	SceneNode* castleArchParentNode;
	Mesh* castleArchMesh;
	MeshMaterial* castleArchMaterial;
	TreePropNode* castleArchPropNode;
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
	
	//--------------------------------------------------------------------
	//Crystals
	SceneNode* crystals1ParentNode;
	SceneNode* crystals2ParentNode;

	Mesh* crystal1Mesh;
	Mesh* crystal2Mesh;

	MeshMaterial* crystal1Material;
	MeshMaterial* crystal2Material;

	TreePropNode* crystal1PropNode;
	TreePropNode* crystal2PropNode;
	//--------------------------------------------------------------------
	
	//Monsters
	Shader* skeletalAnimShader;
	
	SceneNode* monsterDudeParentNode;
	Mesh* monsterDudeMesh;
	MeshAnimation* monsterDudeAnim;
	MeshMaterial* monsterDudeMaterial;
	AnimMeshNode* monsterDudeNode;

	SceneNode* monsterCrabParentNode;
	Mesh* monsterCrabMesh;
	MeshAnimation* monsterCrabAnim;
	MeshMaterial* monsterCrabMaterial;
	AnimMeshNode* monsterCrabNode;

	void NewAnimNodeProp(Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, SceneNode* parent, bool isTransparent = false);
	void NewAnimNodeProp(Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent, bool isTransparent = false);
	void LoadAnimNodeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, SceneNode* parent, bool isTransparent = false);

	//--------------------------------------------------------------------
	// 
	//--------------------------------------------------------------------

	//Lights
	DirectionalLight* dirLight;
	int numPointLights = 1;
	std::vector<Light> allPointLights;
	void CreateNewPointLight();
	//Light* allPointLights;

	//--------------------------------------------------------------------
	
	//--------------------------------------------------------------------

	//Cubemap
	Mesh* quad;
	Skybox* skybox;
	void DrawSkybox();

	//Water
	Shader* reflectShader;
	GLuint waterTex;
	Vector3 waterPosition = Vector3(0, 0, 0);
	void DrawWater();

	//Timer
	GameTimer* timer;

	//Fog
	bool enableFog = true;
	Vector4 fogColour = Vector4();

	//void DrawMainTerrain();
	void DrawNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNode(SceneNode* n, bool includingChild = true);

	std::vector<SceneNode*> transparentNodesList;
	std::vector<SceneNode*> opaqueNodesList;
};