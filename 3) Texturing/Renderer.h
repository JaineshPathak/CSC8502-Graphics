#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer 
{

public:
	Renderer(Window& parent);
	virtual ~Renderer(void);
	void RenderScene()				override;
	void UpdateScene(float msec)	override;

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();

protected:
	float deltaTime;

	Mesh* triangle;
	Shader* shader;

	GLuint texture0;
	GLuint texture1;
	bool filtering;
	bool repeating;

	Camera* camera;
};
