#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../Third Party/imgui/imgui.h"
#include "../Third Party/imgui/imgui_impl_opengl3.h"
#include "../Third Party/imgui/imgui_impl_win32.h"

#include <memory>

class AssetManager;
class Camera;
class SceneNode;
class TerrainNode;
class TreePropNode;
class LightPointNode;
class MeshMaterial;
class MeshAnimation;
class DirectionalLight;
class Skybox;
class ShadowBuffer;
class UniformBuffer;

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

struct PointLightStruct
{
	Vector4 lightPosition;
	Vector4 lightColor;
	Vector4 lightRadiusIntensityData;	//x = Radius, y = Intensity
};

struct DirectionalLightStruct
{
	Vector4 lightDirection;
	Vector4 lightColor;
};

struct EnvironmentDataStruct
{
	Vector4 fogData;	//x = Fog Enabled, y = Density, z = Gradient, 1.0
	Vector4 fogColor;	//Color of Fog

	EnvironmentDataStruct() :
		fogData(Vector4(1.0f, 0.00015f, 1.5f, 1.0f)),
		fogColor(Vector4())
	{
	}

	EnvironmentDataStruct(const bool& fogEnabled, const float& fogDensity, const float& fogGradient, const Vector4 fogColor) :
		fogData(Vector4((float)fogEnabled, fogDensity, fogGradient, 1.0f)),
		fogColor(fogColor)
	{
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
	bool InitBuffers();
	bool InitLights();
	bool InitSkybox();
	bool InitGLParameters();
	bool InitSceneNodes();

	void UpdateUBOData();

	void SpawnSceneNode(const SceneNodeProperties& nodeProp);
	void SpawnSceneNode(const AnimSceneNodeProperties& nodeProp);
	
	void DrawShadowDepth();
	void DrawAllNodes(const bool& isDepth = false);
	void BuildNodeLists(SceneNode* fromNode);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawImGui();

	void DrawNode(SceneNode* Node);
	void DrawDepthNode(SceneNode* Node, bool isTransparentNodes = false);
	void DrawQuadScreen();

private:
	static SceneRenderer* m_Instance;
	AssetManager& m_AssetManager;

	//Camera
	std::shared_ptr<Camera> m_Camera;

	//Meshes
	std::shared_ptr<Mesh> m_CubeMesh;
	std::shared_ptr<Mesh> m_QuadMesh;
	std::shared_ptr<Mesh> m_QuadMiniMesh;
	std::shared_ptr<Mesh> m_PointMesh;

	//Shaders
	std::shared_ptr<Shader> m_TerrainShader;
	std::shared_ptr<Shader> m_DiffuseShader;
	std::shared_ptr<Shader> m_DiffuseAnimShader;
	std::shared_ptr<Shader> m_SkyboxShader;
	std::shared_ptr<Shader> m_DepthShadowShader;
	std::shared_ptr<Shader> m_QuadShader;
	std::shared_ptr<Shader> m_ReflectShader;

	//Lights
	std::shared_ptr<DirectionalLight> m_DirLight;
	DirectionalLightStruct m_DirLightStruct;
	
	std::vector<std::shared_ptr<LightPointNode>> m_PointLightsList;
	std::vector<PointLightStruct> m_PointLightStructList;
	int m_PointLightsNum;

	//Lights Uniform Buffers
	std::shared_ptr<UniformBuffer> m_MatrixUBO;
	std::shared_ptr<UniformBuffer> m_DirLightUBO;
	std::shared_ptr<UniformBuffer> m_PointLightUBO;	
	
	//Skybox and Shadow Buffers
	std::shared_ptr<Skybox> m_Skybox;
	std::shared_ptr<ShadowBuffer> m_ShadowBuffer;

	//Environment Uniform Buffers (Used for Fog)
	EnvironmentDataStruct m_EnvironmentDataStruct;
	std::shared_ptr<UniformBuffer> m_EnvironmentUBO;

	static std::shared_ptr<SceneNode> m_RootNode;
	std::shared_ptr<TerrainNode> m_TerrainNode;

	std::vector<SceneNode*> m_OpaqueNodesList;
	std::vector<SceneNode*> m_TransparentNodesList;

	Matrix4 m_LightSpaceProj;
	Matrix4 m_LightSpaceView;
	Matrix4 m_LightSpaceMatrix;

	float m_OrthographicFOV;

public:
	inline std::shared_ptr<SceneNode> GetRootNode() const { return m_RootNode; }
	inline std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

	inline std::shared_ptr<DirectionalLight> GetDirLight() const { return m_DirLight; }
	inline std::vector<std::shared_ptr<LightPointNode>> GetPointLightsList() const { return m_PointLightsList; }

	inline std::shared_ptr<Mesh> GetQuadMesh() const { return m_QuadMesh; }
	inline std::shared_ptr<Mesh> GetCubeMesh() const { return m_CubeMesh; }
	inline std::shared_ptr<Mesh> GetPointMesh() const { return m_PointMesh; }
	
	unsigned int GetDepthTexture() const;
};