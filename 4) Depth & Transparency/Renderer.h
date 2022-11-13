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

	void ToggleObject();
	void ToggleDepth();
	void ToggleAlphaBlend();
	void ToggleBlendMode();
	void MoveObject(float by);

protected:
	GLuint textures[2];
	Mesh* meshes[2];
	Shader* shader;
	Vector3 positions[2];

	//Plane
	Mesh* planeMesh;
	GLuint planeTexture;

	//Solid Mesh
	Mesh* solidMesh;
	GLuint solidTexture;
	
	bool modifyObject;
	bool usingDepth;
	bool usingAlpha;
	int blendMode;

	Camera* camera;

	//Gun
	/*Mesh* fpsGun;
	Vector3 fpsGunPosition;
	Vector3 fpsGunPositionOffset = Vector3(0.2f, -0.2f, -0.55f);
	Vector3 fpsGunRotation;*/
};