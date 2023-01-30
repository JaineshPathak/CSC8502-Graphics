#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "TerrainNode.h"
//#include "RockNode.h"
#include "TreePropNode.h"
#include "AnimMeshNode.h"
#include "FileHandler.h"
#include "Skybox.h"
#include "CameraPathsManager.h"

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
	bool enableAutoCameraPaths = false;
	CameraPathsManager* cameraPathManager;
	unsigned int total = 0, display = 0;

	Shader* basicDiffuseShader;
	Shader* basicDiffuseShaderInstanced;
	Shader* depthQuadShader;
	Shader* unlitDiffuseShader;
	Shader* skeletalAnimShader;

	float boundingRadiusMultiplier;
	SceneNode* rootNode;
	TerrainNode* terrainNode;
	//TerrainHeightmap* terrainMain;
	Vector3 terrainHeightmapSize;

	//--------------------------------------------------------------------
	//Rocks

	SceneNode* rocks2ParentNode;
	Mesh* rock2Mesh;
	MeshMaterial* rockMaterial;
	int rock2Amount;
	std::vector<GLuint> rockMatTextures;
	std::vector<GLuint> rockMatBumpTextures;
	
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

	void LoadPropData(const std::string& propFilename, Mesh* m, int& propAmount, const float& extraScale = 5.0f);
	void DrawAllInstances();
	void DrawInstancedMesh(Mesh* mesh, GLuint texID, GLuint unit, const std::string& uniformName, Shader* s, int amount);
	void DrawInstancedMesh(Mesh* mesh, Shader* s, const std::vector<GLuint>& matTexturesV, const std::vector<GLuint>& matTexturesBumpV, const int& amount);

	void SetupMeshTextures(Mesh* m, MeshMaterial* meshMaterial, std::vector<GLuint>& matTexturesV, std::vector<GLuint>& matTexturesBumpV);
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
	int castleArchAmount;
	std::vector<GLuint> castleArchMatTextures;
	std::vector<GLuint> castleArchMatBumpTextures;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//Castle Pillar
	SceneNode* castleBridgeParentNode;
	Mesh* castleBridgeMesh;
	MeshMaterial* castleBridgeMaterial;
	TreePropNode* castleBridgePropNode;
	int castlePillarAmount;
	std::vector<GLuint> castlePillarMatTextures;
	std::vector<GLuint> castlePillarMatBumpTextures;
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
	int crystal1Amount;
	int crystal2Amount;

	std::vector<GLuint> crystal1MatTextures;
	std::vector<GLuint> crystal1MatBumpTextures;
	
	std::vector<GLuint> crystal2MatTextures;
	std::vector<GLuint> crystal2MatBumpTextures;

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
	int numPointLights = 1, currentLightIndex;
	std::vector<Light> allPointLights;
	void CreateNewPointLight();

	//--------------------------------------------------------------------
	
	//--------------------------------------------------------------------

	//Cubemap
	Mesh* quad;
	Mesh* cube;
	Skybox* skybox;
	void DrawSkybox();

	//Water
	Shader* reflectShader;
	GLuint waterTex;
	GLuint waterBump;
	Vector3 waterPosition = Vector3(0, 0, 0);
	float waterRotate;
	float waterCycle;
	void DrawWater();

	//Timer
	GameTimer* timer;

	//Fog
	bool enableFog = true;
	Vector4 fogColour = Vector4();

	//Frame Frustum
	Frustum frameFrustum;

	//Shadows
	Matrix4 lightSpaceMatrix;
	Vector3 lightLookAtPos;
	GLuint shadowTex;
	GLuint shadowFBO;
	Shader* shadowShader;
	float zNear = 1.0f, zFar = 25.0f, zLeft = -25.0f, zRight = 25.0f, zTop = -25.0f, zBottom = 25.0f;
	void DrawShadowScene();

	//void DrawMainTerrain();
	void DrawNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNode(SceneNode* n, bool includingChild = true);

	std::vector<SceneNode*> transparentNodesList;
	std::vector<SceneNode*> opaqueNodesList;

	/*unsigned int sampleFBO;
	unsigned int sampleFBOTex;
	unsigned int sampleRBO;*/


	Shader* sceneShader;
	Shader* processShader;

	int blurAmount = 5;
	unsigned int bufferDepthTex;
	unsigned int bufferColorTex[2];
	unsigned int bufferFBO;
	unsigned int bufferPostProcessFBO;

	void PostProcessStage();
	void FinalRender();
};