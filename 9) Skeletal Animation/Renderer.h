#pragma once
#include "..\nclgl\OGLRenderer.h"

class Camera;
class Mesh;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	Camera* camera;
	Mesh* mesh;
	Shader* shader;
	MeshAnimation* anim;
	MeshMaterial* material;
	std::vector<GLuint> matTextures;

	int currentFrame;
	float frameTime;
	float elaspedTime;

	std::vector<Vector3> framePosList;

	//const Matrix4* GetInterpolatedAnim(Vector3 a, Vector3 b);
};