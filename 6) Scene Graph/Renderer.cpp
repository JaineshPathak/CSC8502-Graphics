#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");

	shader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	if (!shader->LoadSuccess())
		return;

	projMatrix = Matrix4::Perspective(0.01f, 10000.0f, (float)width / (float)height, 45.0f);
	
	camera = new Camera();
	camera->SetDefaultSpeed(90.0f);
	camera->SetPosition(Vector3(0, 50, 150));

	root = new SceneNode();
	root->AddChild(new CubeRobot(cube));

	glEnable(GL_DEPTH_TEST);
	init = true;
}

Renderer::~Renderer(void)
{
	delete root;
	delete shader;
	delete camera;
	delete cube;
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	root->Update(dt);
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex0"), 1);

	DrawNode(root);
	//for (size_t i = 0; i < 10; i++)
		//DrawNode(Vector3(i * 35.0f, 0, 0), root);
}

void Renderer::UpdateRobotScale(float s)
{
	root->SetTransform(root->GetTransform() * Matrix4::Scale(Vector3(s, s, s)));
	//root->SetModelScale( root->GetModelScale() + Vector3(s, s, s));
}

void Renderer::DrawNode(SceneNode* n)
{
	/*if (n->GetMesh())
	{
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 0);

		n->Draw(*this);
	}*/

	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 0);

	n->Draw(*this);

	for (std::vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);
}

void Renderer::DrawNode(const Vector3& newLocation, SceneNode* n)
{
	Matrix4 model = Matrix4::Translation(newLocation) * n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 0);

	n->Draw(*this);

	for (std::vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(newLocation, *i);
}