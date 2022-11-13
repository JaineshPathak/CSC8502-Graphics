#include "Renderer.h"

#define RED Vector4(1.0f, 0.0f, 0.0f, 1.0f)
#define GREEN Vector4(0.0f, 1.0f, 0.0f, 1.0f)
#define BLUE Vector4(0.0f, 0.0f, 1.0f, 1.0f)
#define WHITE Vector4(1.0f, 1.0f, 1.0f, 1.0f)

Renderer::Renderer(Window &parent) : OGLRenderer(parent)
{
	//circlesBro = Mesh::GenerateCircle(0.0f, 0.0f, 0.5f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	circleMain = Mesh::GenerateCircle(0.0f, 0.0f, radius, RED, GL_TRIANGLE_FAN, circleVerticesNum);
	circleMainOutline = Mesh::GenerateCircle(0.0f, 0.0f, 0.5f, WHITE, GL_LINE_LOOP, circleVerticesNum);

	//r = 0.5f;

	//For getting points on circle circumference
	for (int i = 0; i < circleVerticesNum; i++)
	{
		float theta = 2.0f * 3.14 * float(i) / float(circleVerticesNum);
		float x = radius * cos(theta);
		float y = radius * sin(theta);

		circlesDots[i] = Mesh::GenerateCircle(x, y, 0.015f, WHITE, GL_TRIANGLE_FAN, circleVerticesNum);
		circlesLines[i] = Mesh::GenerateCircle(x, y, 0.5f, WHITE, GL_LINE_LOOP, circleVerticesNum);
	}

	basicShader = new Shader("basicVertex.glsl", "colourFragment.glsl");

	if(!basicShader->LoadSuccess()) {
		return;
	}

	init = true;
}
Renderer::~Renderer(void)	{
	delete circleMain;
	delete circleMainOutline;
	delete[] circlesDots;
	delete[] circlesLines;
	delete basicShader;
}

void Renderer::UpdateScene(float dt) 
{
	//r += 0.01f;
	//std::cout << r << "\n";
}

void Renderer::RenderScene()	{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);	

	BindShader(basicShader);
	//circlesBro->Draw();

	fakeTimer += 0.001f;
	radius = 0.5f * (1.0f + sin(2.0f * 3.14f * (fakeTimer * 0.5f)));
	//std::cout << r << "\n";

	Vector4 color = Vector4(radius, 0.0f, 0.0f, 1.0f);
	delete circleMain; circleMain = NULL;
	circleMain = Mesh::GenerateCircle(0.0f, 0.0f, radius, color, GL_TRIANGLE_FAN, circleVerticesNum);

	circleMain->Draw();
	circleMainOutline->Draw();
	for (int i = 0; i < circleVerticesNum; i++)
	{
		circlesLines[i]->Draw();
		circlesDots[i]->Draw();
	}
}