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
	
	//------------------------------------------------------------------
	//Rocks
	rocks2ParentNode = new SceneNode();
	rocks2ParentNode->nodeName = "Rocks2Parent";
	rocks5aParentNode = new SceneNode();
	rocks5aParentNode->nodeName = "Rocks5aParent";
	//------------------------------------------------------------------

	//------------------------------------------------------------------
	//Trees
	treesParentNode = new SceneNode();
	treesParentNode->nodeName = "TreesParent";
	//------------------------------------------------------------------

	//------------------------------------------------------------------
	// Castle
	castleParentNode = new SceneNode();
	castleParentNode->nodeName = "CastleParent";
	//------------------------------------------------------------------

	//Ruins
	ruinsParentNode = new SceneNode();
	ruinsParentNode->nodeName = "RuinsParent";

	
	rootNode->AddChild(terrainNode);
	terrainNode->AddChild(rocks2ParentNode);
	terrainNode->AddChild(rocks5aParentNode);
	terrainNode->AddChild(treesParentNode);
	terrainNode->AddChild(castleParentNode);
	terrainNode->AddChild(ruinsParentNode);

	terrainHeightmapSize = terrainNode->GetHeightmapSize();
	cameraMain->SetPosition(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f));

#pragma region OLD
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
#pragma endregion

	basicDiffuseShader = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedVertex.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragment.glsl");
	
	//Rocks
	rock2Mesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Rocks/Mesh_Rock2.msh");
	rock5aMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Rocks/Mesh_Rock5A.msh");
	rockTexture = SOIL_load_OGL_texture(TEXTUREDIRCOURSE"Rocks/Rock5_D.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	LoadRockData(ROCK2FILE, rock2Mesh, rockTexture, rocks2ParentNode);		//Rock2
	LoadRockData(ROCK5AFILE, rock5aMesh, rockTexture, rocks5aParentNode);		//Rock5a

	//Trees
	treeMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Trees/Tree_01.msh");
	treeMaterial = new MeshMaterial(MESHDIRCOURSE"Trees/Tree_01.mat", true);
	LoadTreeData(TREESFILE, treeMesh, treeMaterial, treesParentNode);

	//Castle
	castleMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Castle/Mesh_CastleMain.msh");
	castleMaterial = new MeshMaterial(MESHDIRCOURSE"Castle/Mesh_CastleMain.mat", true);
	LoadTreeData(CASTLEFILE, castleMesh, castleMaterial, castleParentNode);

	//Ruins
	ruinsMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Ruins/Mesh_RuinsMain.msh");
	ruinsMaterial = new MeshMaterial(MESHDIRCOURSE"Ruins/Mesh_RuinsMain.mat", true);
	LoadTreeData(RUINSFILE, ruinsMesh, ruinsMaterial, ruinsParentNode);

	//std::cout << "Parent Scale: " << rocksParentNode->GetModelScale() << "\n";
	//std::cout << "Child Scale: " << rockNode->GetModelScale() << "\n";

	projMatrix = Matrix4::Perspective(1.0, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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



//Init at Center of map
void Renderer::NewRock(Mesh* m, GLuint t, SceneNode* parent)
{
	RockNode* rock = new RockNode();
	rock->SetShader(basicDiffuseShader);
	rock->SetTexture(t);
	rock->SetMesh(m);
	rock->SetModelScale(Vector3(3.5f, 6.5f, 3.5f));
	rock->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * rock->GetRotationMatrix() * Matrix4::Scale(rock->GetModelScale()));
	parent->AddChild(rock);
}

//Read from file
void Renderer::NewRock(Mesh* m, GLuint t, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent)
{
	RockNode* rock = new RockNode();
	rock->SetShader(basicDiffuseShader);
	rock->SetTexture(t);
	rock->SetMesh(m);
	rock->SetModelRotation(Rot);
	rock->SetModelScale(Scale);
	rock->SetTransform(Matrix4::Translation(Pos) * rock->GetRotationMatrix() * Matrix4::Scale(rock->GetModelScale()));
	parent->AddChild(rock);
}

void Renderer::LoadRockData(const std::string& fileName, Mesh* m, GLuint t, SceneNode* parent)
{
	if (FileHandler::FileExists(fileName))
	{
		std::vector<Vector3> posV, rotV, scaleV;
		FileHandler::ReadPropDataFromFile(fileName, posV, rotV, scaleV);

		for (int i = 0; i < posV.size(); i++)
			NewRock(m, t, posV[i], rotV[i], scaleV[i], parent);
	}
	else
		NewRock(m, t, parent);
}



void Renderer::NewTreeProp(Mesh* m, MeshMaterial* mMat, SceneNode* parent)
{
	TreePropNode* tree = new TreePropNode(m, mMat, basicDiffuseShader, TEXTUREDIR);
	tree->SetModelScale(Vector3(6.0f, 6.0f, 6.0f));
	tree->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * tree->GetRotationMatrix() * Matrix4::Scale(tree->GetModelScale()));
	parent->AddChild(tree);
}

void Renderer::NewTreeProp(Mesh* m, MeshMaterial* mMat, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent)
{
	TreePropNode* tree = new TreePropNode(m, mMat, basicDiffuseShader, TEXTUREDIR);
	tree->SetModelRotation(Rot);
	tree->SetModelScale(Scale);
	tree->SetTransform(Matrix4::Translation(Pos) * tree->GetRotationMatrix() * Matrix4::Scale(tree->GetModelScale()));
	parent->AddChild(tree);
}

void Renderer::LoadTreeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, SceneNode* parent)
{
	if (FileHandler::FileExists(fileName))
	{
		std::vector<Vector3> posV, rotV, scaleV;
		FileHandler::ReadPropDataFromFile(fileName, posV, rotV, scaleV);

		for (int i = 0; i < posV.size(); i++)
			NewTreeProp(m, mMat, posV[i], rotV[i], scaleV[i], parent);
	}
	else
		NewTreeProp(m, mMat, parent);
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
		if (ImGui::DragFloat3(_labelPrefix("Rotation").c_str(), (float*)&cRot))
			cameraMain->SetRotation(cRot);
		if (ImGui::DragFloat(_labelPrefix("Speed").c_str(), &cSpeed))
			cameraMain->SetDefaultSpeed(cSpeed);
	}
	if (ImGui::CollapsingHeader("Rocks2"))
	{
		for (int i = 0; i < rocks2ParentNode->GetChildCount() && (rocks2ParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = rocks2ParentNode->GetChild(i);
			ImGui::Text((std::string("Rock[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			Vector3 nodeRotation = node->GetModelRotation();

			//Position
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			
			//Rotation
			if (ImGui::DragFloat3(_labelPrefix(std::string("Rotation[" + indexStr + "]").c_str()).c_str(), (float*)&nodeRotation))
			{
				node->SetModelRotation(nodeRotation);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			//Scaling
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}

		if (ImGui::Button("New Rock2"))
			NewRock(rock2Mesh, rockTexture, rocks2ParentNode);

		ImGui::SameLine();
		if (ImGui::Button("Save Rock2.sav"))
		{
			std::vector<Vector3> rock2PosV, rock2RotV, rock2ScaleV;
			for (int i = 0; i < rocks2ParentNode->GetChildCount() && (rocks2ParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = rocks2ParentNode->GetChild(i);
				rock2PosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				rock2RotV.emplace_back(node->GetModelRotation());
				rock2ScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(ROCK2FILE, rock2PosV, rock2RotV, rock2ScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Rocks5A"))
	{
		for (int i = 0; i < rocks5aParentNode->GetChildCount() && (rocks5aParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = rocks5aParentNode->GetChild(i);
			ImGui::Text((std::string("Rock[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			Vector3 nodeRotation = node->GetModelRotation();

			//Position
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));

			//Rotation
			if (ImGui::DragFloat3(_labelPrefix(std::string("Rotation[" + indexStr + "]").c_str()).c_str(), (float*)&nodeRotation))
			{
				node->SetModelRotation(nodeRotation);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			//Scaling
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}

		if (ImGui::Button("New Rock5A"))
			NewRock(rock5aMesh, rockTexture, rocks5aParentNode);

		ImGui::SameLine();
		if (ImGui::Button("Save Rock5A.sav"))
		{
			std::vector<Vector3> rock5aPosV, rock5aRotV, rock5aScaleV;
			for (int i = 0; i < rocks5aParentNode->GetChildCount() && (rocks5aParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = rocks5aParentNode->GetChild(i);
				rock5aPosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				rock5aRotV.emplace_back(node->GetModelRotation());
				rock5aScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(ROCK5AFILE, rock5aPosV, rock5aRotV, rock5aScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Trees"))
	{
		for (int i = 0; i < treesParentNode->GetChildCount() && (treesParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = treesParentNode->GetChild(i);
			ImGui::Text((std::string("Tree[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			Vector3 nodeRotation = node->GetModelRotation();

			//Position
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));

			//Rotation
			if (ImGui::DragFloat3(_labelPrefix(std::string("Rotation[" + indexStr + "]").c_str()).c_str(), (float*)&nodeRotation))
			{
				node->SetModelRotation(nodeRotation);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			//Scaling
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}

		if (ImGui::Button("New Tree Prop"))
			NewTreeProp(treeMesh, treeMaterial, treesParentNode);

		ImGui::SameLine();
		if (ImGui::Button("Save TreesProp.sav"))
		{
			std::vector<Vector3> treePosV, treeRotV, treeScaleV;
			for (int i = 0; i < treesParentNode->GetChildCount() && (treesParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = treesParentNode->GetChild(i);
				treePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				treeRotV.emplace_back(node->GetModelRotation());
				treeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(TREESFILE, treePosV, treeRotV, treeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Castle"))
	{
		for (int i = 0; i < castleParentNode->GetChildCount() && (castleParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = castleParentNode->GetChild(i);
			ImGui::Text((std::string("Castle[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			Vector3 nodeRotation = node->GetModelRotation();

			//Position
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));

			//Rotation
			if (ImGui::DragFloat3(_labelPrefix(std::string("Rotation[" + indexStr + "]").c_str()).c_str(), (float*)&nodeRotation))
			{
				node->SetModelRotation(nodeRotation);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			//Scaling
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}		

		ImGui::SameLine();
		if (ImGui::Button("Save CastleProp.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < castleParentNode->GetChildCount() && (castleParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = castleParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CASTLEFILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Ruins"))
	{
		for (int i = 0; i < ruinsParentNode->GetChildCount() && (ruinsParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = ruinsParentNode->GetChild(i);
			ImGui::Text((std::string("Ruin[") + indexStr + "]:").c_str());

			Vector3 nodePos = node->GetWorldTransform().GetPositionVector();
			Vector3 nodeScale = node->GetModelScale();
			Vector3 nodeRotation = node->GetModelRotation();

			//Position
			if (ImGui::DragFloat3(_labelPrefix(std::string("Position[" + indexStr + "]").c_str()).c_str(), (float*)&nodePos))
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));

			//Rotation
			if (ImGui::DragFloat3(_labelPrefix(std::string("Rotation[" + indexStr + "]").c_str()).c_str(), (float*)&nodeRotation))
			{
				node->SetModelRotation(nodeRotation);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			//Scaling
			if (ImGui::DragFloat3(_labelPrefix(std::string("Scale[" + indexStr + "]").c_str()).c_str(), (float*)&nodeScale))
			{
				node->SetModelScale(nodeScale);
				node->SetTransform(Matrix4::Translation(nodePos) * node->GetRotationMatrix() * Matrix4::Scale(node->GetModelScale()));
			}

			ImGui::Separator();
		}

		ImGui::SameLine();
		if (ImGui::Button("Save RuinsProp.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < ruinsParentNode->GetChildCount() && (ruinsParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = ruinsParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(RUINSFILE, nodePosV, nodeRotV, nodeScaleV);
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

#pragma region OLD
//void Renderer::DrawMainTerrain()
//{
//	BindShader(terrainNode->GetTerrainShader());
//	//modelMatrix.ToIdentity();
//	UpdateShaderMatrices();
//	//terrainNode->BindTerrainShader();
//
//	/*glUniform1i(glGetUniformLocation(terrainMain->GetTerrainShader()->GetProgram(), "diffuseGrassTex"), 0);
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, terrainMain->GetTerrainTextureGrass());*/
//}  
#pragma endregion

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

//void Renderer::DrawRocks2()
//{
//	BindShader(rocksShader);
//	UpdateShaderMatrices();
//	OGLRenderer::BindTexture(rockNode->GetTexture(), 0, "diffuseTex", rocksShader);
//	rockNode->Draw(*this);
//}