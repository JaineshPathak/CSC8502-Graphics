#include "Renderer.h"
#include <map>

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	camera = new Camera();

	solidMesh = Mesh::GenerateCube();
	//Mesh::SetAllVertexColour(solidMesh, Vector4());		//White Default Color
	solidTexture = SOIL_load_OGL_texture(TEXTUREDIR"rusted_down.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	planeMesh = Mesh::GenerateQuad();
	Mesh::SetAllVertexColour(planeMesh, Vector4());		//White Default Color
	planeTexture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	meshes[0] = Mesh::GenerateQuad();
	meshes[1] = Mesh::GenerateQuad();

	Mesh::SetAllVertexColour(meshes[0], Vector4());
	Mesh::SetAllVertexColour(meshes[1], Vector4());

	textures[0] = SOIL_load_OGL_texture(TEXTUREDIR"blending_transparent_window.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	textures[1] = SOIL_load_OGL_texture(TEXTUREDIR"blending_transparent_window.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!textures[0] || !textures[1])
		return;

	positions[0] = Vector3(0, 0, -5);
	positions[1] = Vector3(0, 0, -5);

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!shader->LoadSuccess())
		return;

	usingDepth = false;
	usingAlpha = false;
	blendMode = 0;
	modifyObject = true;

	projMatrix = Matrix4::Perspective(0.01f, 10000.0f, (float)width / (float)height, 45.0f);

	init = true;
}

Renderer::~Renderer(void)
{
	delete planeMesh;
	delete meshes[0];
	delete meshes[1];
	delete shader;

	glDeleteTextures(1, &planeTexture);
	glDeleteTextures(1, &solidTexture);
	glDeleteTextures(2, textures);
}

void Renderer::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);

	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex0"), 0);
	glActiveTexture(GL_TEXTURE0);

	Matrix4 planeMatrix = Matrix4::Translation(Vector3(0, -0.5f, -5.0f)) * Matrix4::Rotation(-90.0f, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(10, 10, 10));
	glBindTexture(GL_TEXTURE_2D, planeTexture);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, planeMatrix.values);
	planeMesh->Draw();

	Matrix4 solidMeshMatrix = Matrix4::Translation(Vector3(0, 0, -9));
	glBindTexture(GL_TEXTURE_2D, solidTexture);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, solidMeshMatrix.values);
	solidMesh->Draw();

	std::map<float, Vector3> sorted;
	for (unsigned int i = 0; i < 2; i++)
	{
		float distance = (camera->getPosition() - positions[i]).Length();
		sorted[distance] = positions[i];
	}

	unsigned int i = 0;
	for (std::map<float, Vector3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
	{		
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, (float*)&Matrix4::Translation(it->second));
		meshes[i]->Draw();

		i++;
	}
}

void Renderer::ToggleObject() 
{
	 modifyObject = !modifyObject;
}

void Renderer::MoveObject(float by) 
{
	positions[(int)modifyObject].z += by;
}

void Renderer::ToggleBlendMode() 
{
	blendMode = (blendMode + 1) % 4;
	std::cout << "Blend Mode: " << blendMode << "\n";
	switch (blendMode) 
	{
		case (0): glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;		//Uses Image alpha
		case (1): glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR); break;		//Removes Black color pixels
		case (2): glBlendFunc(GL_ONE, GL_ZERO); break;							//Source always, Dest never
		case (3): glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;						//Source alpha, Dest always (opaque)
	};
}

void Renderer::ToggleDepth() 
{
	usingDepth = !usingDepth;
	usingDepth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void Renderer::ToggleAlphaBlend() 
{
	usingAlpha = !usingAlpha;
	usingAlpha ? glEnable(GL_BLEND) : glDisable(GL_BLEND);	
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}