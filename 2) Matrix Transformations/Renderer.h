#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float dt) override;

	void SwitchToPerspective();
	void SwitchToOrthographic();

	inline void SetScale(float s) { scale = s; }
	inline void SetRotation(float r) { rotation = r; }
	inline void SetPosition(Vector3 p) { position = p; }
	inline void SetFOV(float _fov) { fov = _fov; }

protected:
	Mesh* triangle;
	Shader* matrixShader;
	float scale;
	float rotation;
	float fov;
	Vector3 position;

	Camera* camera;

	//Gun
	Mesh* fpsGun;
	Vector3 fpsGunPosition;
	Vector3 fpsGunPositionOffset = Vector3(0.2f, -0.2f, -0.55f);
	Vector3 fpsGunRotation;
};