#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "Rules.h"

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;
protected:
	float radius = 0.5f;
	Mesh*	circleMain;
	Mesh* circlesDots[circleVerticesNum];
	Mesh* circleMainOutline;
	Mesh* circlesLines[circleVerticesNum];
	Shader* basicShader;

	float fakeTimer = 0.0f;
};
