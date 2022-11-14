#include "Renderer.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)
{
	cameraMain = new Camera();
	cameraMain->SetDefaultSpeed(450.0f);

	rootNode = new SceneNode();
	rootNode->nodeName = "Root";

	terrainNode = new TerrainNode();
	
	rocksParentNode = new SceneNode();
	rocksParentNode->nodeName = "RocksParent";
	
	treesParentNode = new SceneNode();
	treesParentNode->nodeName = "TreesParent";
	
	rootNode->AddChild(terrainNode);
	terrainNode->AddChild(rocksParentNode);
	terrainNode->AddChild(treesParentNode);

	Vector3 terrainHeightmapSize = terrainNode->GetHeightmapSize();
	cameraMain->SetPosition(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f));

	/*rocksMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Rocks/Mesh_Rock2.msh");
	rocksShader = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedVertex.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragment.glsl");
	rocksTexture = SOIL_load_OGL_texture(TEXTUREDIRCOURSE"Rocks/Rock5_D.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	rockNode = new SceneNode();
	rockNode->nodeName = "Rock01";
	rockNode->SetMesh(rocksMesh);
	rockNode->SetTransform(Matrix4::Translation(Vector3(871.2f, 117.5f, 418.5f)));
	rockNode->SetModelScale(Vector3(3.5f, 7.5f, 3.5f));
	rockNode->SetTexture(rocksTexture);
	rockNode->SetShader(rocksShader);*/

	rockNode = new RockNode();
	rockNode->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f)));
	rockNode->SetModelScale(Vector3(3.5f, 7.5f, 3.5f) * 2.5f);
	rocksParentNode->AddChild(rockNode);

	//std::cout << "Parent Scale: " << rocksParentNode->GetModelScale() << "\n";
	//std::cout << "Child Scale: " << rockNode->GetModelScale() << "\n";

	std::cout << "Terrain: " << terrainHeightmapSize;

	projMatrix = Matrix4::Perspective(1.0, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer(void)	
{
	delete cameraMain;
}

void Renderer::UpdateScene(float dt) 
{
	cameraMain->UpdateCamera(dt);
	viewMatrix = cameraMain->BuildViewMatrix();

	rootNode->Update(dt);
}

void Renderer::RenderScene()	
{
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DrawMainTerrain();	
	//UpdateShaderMatrices();
	DrawNode(rootNode);
}

void Renderer::DrawMainTerrain()
{
	BindShader(terrainNode->GetTerrainShader());
	//modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	//terrainNode->BindTerrainShader();

	/*glUniform1i(glGetUniformLocation(terrainMain->GetTerrainShader()->GetProgram(), "diffuseGrassTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainMain->GetTerrainTextureGrass());*/
}

void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh())
	{
		BindShader(n->GetShader());
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();
		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
		n->Draw(*this);
	}
	
	for (std::vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);
}

void Renderer::DrawRocks()
{
	//BindShader(rocksShader);
	//UpdateShaderMatrices();
	//OGLRenderer::BindTexture(rockNode->GetTexture(), 0, "diffuseTex", rocksShader);
	//rockNode->Draw(*this);
}