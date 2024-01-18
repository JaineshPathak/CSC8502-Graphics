#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <memory>

class AssetManager;
class Camera;
class SceneNode;
class TerrainNode;
class TreePropNode;
class MeshMaterial;
class MeshAnimation;
class DirectionalLight;
class Skybox;

struct SceneNodeProperties
{
	std::string nodeName;
	std::string nodeFilePath;
	std::shared_ptr<Mesh> nodeMesh;
	std::shared_ptr<MeshMaterial> nodeMeshMaterial;
	std::shared_ptr<SceneNode> nodeParent;
	bool isTransparent;

	SceneNodeProperties(std::string nodeName, std::string nodeFilePath, std::shared_ptr<Mesh> nodeMesh, std::shared_ptr<MeshMaterial> nodeMeshMaterial, bool isTransparent)
	{
		this->nodeName = nodeName;
		this->nodeFilePath = nodeFilePath;
		this->nodeMesh = nodeMesh;
		this->nodeMeshMaterial = nodeMeshMaterial;
		this->nodeParent = nullptr;
		this->isTransparent = isTransparent;
	}
};

struct AnimSceneNodeProperties : SceneNodeProperties
{
	std::shared_ptr<MeshAnimation> nodeMeshAnimation;

	AnimSceneNodeProperties(std::string nodeName, std::string nodeFilePath, std::shared_ptr<Mesh> nodeMesh, std::shared_ptr<MeshMaterial> nodeMeshMaterial, std::shared_ptr<MeshAnimation> nodeMeshAnimation, bool isTransparent) :
		SceneNodeProperties(nodeName, nodeFilePath, nodeMesh, nodeMeshMaterial, isTransparent)
	{
		this->nodeParent = nullptr;
		this->nodeMeshAnimation = nodeMeshAnimation;
	}
};

class SceneRenderer : public OGLRenderer
{
public:
	SceneRenderer(Window& parent);
	~SceneRenderer(void);

	static SceneRenderer* Get() { return m_Instance; }

	virtual void RenderScene() override;
	virtual void UpdateScene(float DeltaTime) override;

protected:
	bool Initialize();

	bool InitCamera();
	bool InitShaders();
	bool InitMeshes();
	bool InitMeshMaterials();
	bool InitMeshAnimations();
	bool InitLights();
	bool InitSkybox();
	bool InitGLParameters();
	bool InitSceneNodes();

	void SpawnSceneNode(const SceneNodeProperties& nodeProp);
	void SpawnSceneNode(const AnimSceneNodeProperties& nodeProp);
	
	void DrawAllNodes();
	void BuildNodeLists(SceneNode* fromNode);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNode(SceneNode* Node);

private:
	static SceneRenderer* m_Instance;
	AssetManager& m_AssetManager;

	std::shared_ptr<Mesh> m_CubeMesh;
	std::shared_ptr<Mesh> m_QuadMesh;

	std::shared_ptr<Camera> m_Camera;

	static std::shared_ptr<SceneNode> m_RootNode;
	std::shared_ptr<TerrainNode> m_TerrainNode;

	std::shared_ptr<DirectionalLight> m_DirLight;
	std::vector<Light> m_PointLightsList;
	int m_PointLightsNum;

	std::shared_ptr<Skybox> m_Skybox;

	std::vector<SceneNode*> m_OpaqueNodesList;
	std::vector<SceneNode*> m_TransparentNodesList;

	std::shared_ptr<Shader> m_TerrainShader;
	std::shared_ptr<Shader> m_DiffuseShader;
	std::shared_ptr<Shader> m_DiffuseAnimShader;
	std::shared_ptr<Shader> m_SkyboxShader;

public:
	inline std::shared_ptr<SceneNode> GetRootNode() const { return m_RootNode; }
	inline std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

	inline std::shared_ptr<Mesh> GetQuadMesh() const { return m_QuadMesh; }	
};