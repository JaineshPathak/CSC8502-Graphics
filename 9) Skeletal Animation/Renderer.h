#pragma once
#include "..\nclgl\OGLRenderer.h"
#include "../Third Party/imgui/imgui.h"
#include "../Third Party/imgui/imgui_impl_opengl3.h"
#include "../Third Party/imgui/imgui_impl_win32.h"

#include <thread>
#include <atomic>

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
	void RenderImGui();
	void UpdateScene(float dt) override;

protected:
	Matrix4 LerpMat(const Matrix4& a, const Matrix4& b, float t)
	{
		Matrix4 res;

		for (int i = 0; i < 16; i++)
		{
			res.values[i] = naive_lerp(a.values[i], b.values[i], t);
		}

		return res;
	}

	Camera* camera;
	Mesh* mesh;
	Shader* shader;
	MeshAnimation* anim;
	MeshAnimation* anim2;
	MeshAnimation* jumpAnim;
	MeshAnimation* currentAnim;
	MeshAnimation* previousAnim;
	MeshAnimation* pendingAnim;
	MeshMaterial* material;
	std::vector<GLuint> matTextures;

	Window& w;
	bool showCursor;
	bool playAutomatic;
	bool editMode;

	float blendFactor;
	int currentFrame;
	float frameTime;
	float elaspedTime;

	Matrix4 finalBlend;
	Matrix4 finalBlendInverse;

	std::atomic<float> globalDt;
	
	bool isTweening;
	std::thread tweenThread;
	float tweenTimeCurrent;
	float tweenTime;

	void PlayAnim(MeshAnimation* newAnim);
	void TweenAnim(MeshAnimation* previousAnim, MeshAnimation* nextAnim, float time);
	std::vector<Vector3> framePosList;

	//const Matrix4* GetInterpolatedAnim(Vector3 a, Vector3 b);
};