#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <memory>

class Camera;
class TerrainNode;
class SceneNode;
class DirectionalLight;

class SceneRenderer : public OGLRenderer
{
public:
	SceneRenderer(Window& parent);
	~SceneRenderer(void);

	virtual void RenderScene() override;
	virtual void UpdateScene(float DeltaTime) override;

protected:
	bool Initialize();

	bool InitCamera();
	bool InitMeshes();
	bool InitLights();
	bool InitGLParameters();

	void DrawAllNodes();
	void BuildNodeLists(SceneNode* fromNode);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawNode(SceneNode* Node);

private:
	std::shared_ptr<Mesh> m_CubeMesh;
	std::shared_ptr<Mesh> m_QuadMesh;

	std::shared_ptr<Camera> m_Camera;

	std::shared_ptr<TerrainNode> m_TerrainNode;

	std::shared_ptr<DirectionalLight> m_DirLight;
	std::vector<Light> m_PointLightsList;
	int m_PointLightsNum;

	std::vector<SceneNode*> m_OpaqueNodesList;
	std::vector<SceneNode*> m_TransparentNodesList;

public:
	inline std::shared_ptr<Camera> GetCamera() const { return m_Camera; }
};