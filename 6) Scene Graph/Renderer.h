#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float dt) override;
	void RenderScene() override;

	void UpdateRobotScale(float s);

protected:
	void DrawNode(SceneNode* n);
	void DrawNode(const Vector3& Offset, SceneNode* n);

	SceneNode* root;
	Camera* camera;
	Mesh* cube;
	Shader* shader;
};