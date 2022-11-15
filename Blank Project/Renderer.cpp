#include "Renderer.h"
#include "../nclgl/Camera.h"

static std::string _labelPrefix(const char* const label)
{
	float width = ImGui::CalcItemWidth();

	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::SetNextItemWidth(-1);

	std::string labelID = "##";
	labelID += label;

	return labelID;
}

Renderer::Renderer(Window &parent) : OGLRenderer(parent)
{
	//-----------------------------------------------------------
	//Imgui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
	//-----------------------------------------------------------

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

	for (int i = 0; i < 2; i++)
	{
		RockNode* rock = new RockNode();
		rock->SetModelScale(Vector3(3.5f, 6.5f, 3.5f));
		rock->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * Matrix4::Scale(rock->GetModelScale()));
		rocksParentNode->AddChild(rock);
	}

	//std::cout << "Parent Scale: " << rocksParentNode->GetModelScale() << "\n";
	//std::cout << "Child Scale: " << rockNode->GetModelScale() << "\n";

	projMatrix = Matrix4::Perspective(1.0, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer(void)	
{
	delete cameraMain;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::UpdateScene(float dt) 
{
	cameraMain->UpdateCamera(dt);
	viewMatrix = cameraMain->BuildViewMatrix();

	rootNode->Update(dt);
}

void Renderer::UpdateImGui()
{
	//glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/*ImGui::Begin("Debug Window");
	ImGui::Text("Camera Position: "  cameraMain->getPosition());
	ImGui::End();*/
	if (ImGui::CollapsingHeader("Terrain"))
		ImGui::DragFloat3(_labelPrefix("Heightmap Size").c_str(), (float*)&terrainNode->GetHeightmapSize());
	if (ImGui::CollapsingHeader("Camera"))
	{
		Vector3 cPos = cameraMain->getPosition();
		Vector3 cRot = cameraMain->getRotation();
		float cSpeed = cameraMain->getDefaultSpeed();
		if (ImGui::DragFloat3(_labelPrefix("Position").c_str(), (float*)&cPos))
			cameraMain->SetPosition(cPos);
		if (ImGui::DragFloat3(_labelPrefix("Rotation").c_str(), (float*)&cameraMain->getRotation()))
			cameraMain->SetRotation(cRot);
		if (ImGui::DragFloat(_labelPrefix("Speed").c_str(), &cSpeed))
			cameraMain->SetDefaultSpeed(cSpeed);
	}
	if (ImGui::CollapsingHeader("Rocks"))
	{
		for (int i = 0; i < rocksParentNode->GetChildCount() && (rocksParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = rocksParentNode->GetChild(i);
			ImGui::Text((std::string("Rock[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * Matrix4::Scale(node->GetModelScale()));
			
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}
	}

	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::RenderScene()	
{
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//DrawMainTerrain();	
	//UpdateShaderMatrices();
	DrawNode(rootNode);

	UpdateImGui();
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