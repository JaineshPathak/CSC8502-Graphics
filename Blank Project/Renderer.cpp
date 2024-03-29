#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Light.h"
#include "../nclgl/DirectionalLight.h"
#include <algorithm>

#define SHADOW_SIZE 1024

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
	timer = parent.GetTimer();
	quad = Mesh::GenerateQuad();

	//-----------------------------------------------------------
	//Imgui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
	//-----------------------------------------------------------
	
	InitNodes();
	if (!InitCamera()) return;
	if (!InitShaders()) return;
	if (!InitLights()) return;
	if (!InitMeshes()) return;
	if (!InitMeshMaterials()) return;
	if (!InitMeshAnimations()) return;
	if (!InitSkybox()) return;
	InitData();
	InitWater();
	InitFog();
	SetupShadows();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer(void)
{
	delete cameraMain;
	delete cameraPathManager;

	delete rootNode;
	delete rock2Mesh;
	delete rockMaterial;

	delete treeMesh;
	delete treeMaterial;

	delete castleMesh;
	delete castleMaterial;

	delete castlePillarMesh;
	delete castlePillarMaterial;

	delete castleBridgeMesh;
	delete castleBridgeMaterial;

	delete ruinsMesh;
	delete ruinsMaterial;

	delete crystal1Mesh;
	delete crystal2Mesh;

	delete crystal1Material;
	delete crystal2Material;

	delete monsterDudeMesh;
	delete monsterDudeMaterial;
	delete monsterDudeAnim;

	delete monsterCrabMesh;
	delete monsterCrabMaterial;
	delete monsterCrabAnim;

	delete dirLight;
	delete quad;
	delete skybox;

	glDeleteTextures(1, &waterTex);
	glDeleteTextures(1, &waterBump);

	delete basicDiffuseShader;
	delete skeletalAnimShader;
	delete reflectShader;
	delete shadowShader;

	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	delete timer;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::InitNodes()
{
	//-----------------------------------------------------------
	//Root Node
	rootNode = new SceneNode("Root");

	//Terrain Node
	terrainNode = new TerrainNode();
	terrainHeightmapSize = terrainNode->GetHeightmapSize();

	//------------------------------------------------------------------
	//Rocks
	rocks2ParentNode = new SceneNode("Rocks2Parent");
	//------------------------------------------------------------------

	//------------------------------------------------------------------
	//Trees
	treesParentNode = new SceneNode("TreesParent");
	//------------------------------------------------------------------

	//------------------------------------------------------------------
	// Castle
	castleParentNode = new SceneNode("CastleParent");
	castlePillarParentNode = new SceneNode("CastlePillarsParent");
	castleArchParentNode = new SceneNode("CastleArchParent");
	castleBridgeParentNode = new SceneNode("CastleBridgeParent");
	//------------------------------------------------------------------

	//Ruins
	ruinsParentNode = new SceneNode("RuinsParent");
	//------------------------------------------------------------------

	// Crystals
	crystals1ParentNode = new SceneNode("Crystal01Parent");
	crystals2ParentNode = new SceneNode("Crystal02Parent");
	//------------------------------------------------------------------

	//Monsters
	monsterDudeParentNode = new SceneNode("MonstersDudeParent");
	monsterCrabParentNode = new SceneNode("MonstersCrabParent");
	//------------------------------------------------------------------

	rootNode->AddChild(terrainNode);
	terrainNode->AddChild(rocks2ParentNode);
	terrainNode->AddChild(treesParentNode);
	terrainNode->AddChild(castleParentNode);
	terrainNode->AddChild(castlePillarParentNode);
	terrainNode->AddChild(castleArchParentNode);
	terrainNode->AddChild(castleBridgeParentNode);
	terrainNode->AddChild(ruinsParentNode);
	terrainNode->AddChild(crystals1ParentNode);
	terrainNode->AddChild(crystals2ParentNode);
	terrainNode->AddChild(monsterDudeParentNode);
	terrainNode->AddChild(monsterCrabParentNode);	
}

bool Renderer::InitCamera()
{
	cameraMain = new Camera();
	if (cameraMain == nullptr)
		return false;

	cameraMain->SetDefaultSpeed(850.0f);
	cameraMain->SetPosition(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f));

	cameraPathManager = new CameraPathsManager(enableAutoCameraPaths, CAMERAPATHS, cameraMain);

	return true;
}

bool Renderer::InitLights()
{
	dirLight = new DirectionalLight(Vector3(1, 1, 0), Vector4(1.0f, 0.8703716f, 0.7294118f, 1.0f), Vector4());
	if (dirLight == nullptr)
		return false;

	if (FileHandler::FileExists(LIGHTSDATAFILE))
	{
		FileHandler::ReadLightDataFile(LIGHTSDATAFILE, *dirLight, allPointLights);
		if ((int)allPointLights.size() > 0)
			numPointLights = (int)allPointLights.size();
	}
	else
	{
		for (int i = 0; i < numPointLights && (numPointLights > 0); i++)
			CreateNewPointLight();
	}

	return true;
}

bool Renderer::InitMeshes()
{
	if (!LoadMesh(&rock2Mesh, MESHDIRCOURSE, "Rocks/Mesh_Rock5D.msh")) return false;
	if (!LoadMesh(&treeMesh, MESHDIRCOURSE, "Trees/Tree_01.msh")) return false;
	if (!LoadMesh(&castleMesh, MESHDIRCOURSE, "Castle/Mesh_CastleMain.msh")) return false;
	if (!LoadMesh(&castlePillarMesh, MESHDIRCOURSE, "Castle/Mesh_CastlePillar.msh")) return false;
	if (!LoadMesh(&castleArchMesh, MESHDIRCOURSE, "Castle/Mesh_CastleArch.msh")) return false;
	if (!LoadMesh(&castleBridgeMesh, MESHDIRCOURSE, "Castle/Mesh_Bridge.msh")) return false;
	if (!LoadMesh(&ruinsMesh, MESHDIRCOURSE, "Ruins/Mesh_RuinsMain.msh")) return false;
	if (!LoadMesh(&crystal1Mesh, MESHDIRCOURSE, "Crystals/Mesh_Crystal_01.msh")) return false;
	if (!LoadMesh(&crystal2Mesh, MESHDIRCOURSE, "Crystals/Mesh_Crystal_02.msh")) return false;
	if (!LoadMesh(&monsterDudeMesh, MESHDIRCOURSE, "Monsters/Monster_Dude.msh")) return false;
	if (!LoadMesh(&monsterCrabMesh, MESHDIRCOURSE, "Monsters/Monster_Crab.msh")) return false;

	return true;
}

bool Renderer::InitShaders()
{
	if (!LoadShader(&basicDiffuseShader, SHADERDIRCOURSETERRAIN, "CWTexturedVertexv2.glsl", "CWTexturedFragmentv2.glsl")) return false;
	if (!LoadShader(&skeletalAnimShader, SHADERDIRCOURSETERRAIN, "CWTexturedSkinVertex.glsl", "CWTexturedSkinFragment.glsl")) return false;
	if (!LoadShader(&reflectShader, SHADERDIRCOURSETERRAIN, "CWReflectVertex.glsl", "CWReflectFragment.glsl")) return false;
	if (!LoadShader(&shadowShader, "", "shadowVert.glsl", "shadowFrag.glsl")) return false;

	return true;
}

bool Renderer::InitMeshMaterials()
{
	if (!LoadMeshMaterial(&rockMaterial, MESHDIRCOURSE, "Rocks/Mesh_Rock5D.mat")) return false;
	if (!LoadMeshMaterial(&treeMaterial, MESHDIRCOURSE, "Trees/Tree_01.mat")) return false;
	if (!LoadMeshMaterial(&castleMaterial, MESHDIRCOURSE, "Castle/Mesh_CastleMain.mat")) return false;
	if (!LoadMeshMaterial(&castleArchMaterial, MESHDIRCOURSE, "Castle/Mesh_CastleArch.mat")) return false;
	if (!LoadMeshMaterial(&castlePillarMaterial, MESHDIRCOURSE, "Castle/Mesh_CastlePillar.mat")) return false;
	if (!LoadMeshMaterial(&castleBridgeMaterial, MESHDIRCOURSE, "Castle/Mesh_Bridge.mat")) return false;
	if (!LoadMeshMaterial(&ruinsMaterial, MESHDIRCOURSE, "Ruins/Mesh_RuinsMain.mat")) return false;
	if (!LoadMeshMaterial(&crystal1Material, MESHDIRCOURSE, "Crystals/Mesh_Crystal_01.mat")) return false;
	if (!LoadMeshMaterial(&crystal2Material, MESHDIRCOURSE, "Crystals/Mesh_Crystal_02.mat")) return false;
	if (!LoadMeshMaterial(&monsterDudeMaterial, MESHDIRCOURSE, "Monsters/Monster_Dude.mat")) return false;
	if (!LoadMeshMaterial(&monsterCrabMaterial, MESHDIRCOURSE, "Monsters/Monster_Crab.mat")) return false;

	return true;
}

bool Renderer::InitMeshAnimations()
{
	if (!LoadMeshAnimations(&monsterDudeAnim, MESHDIRCOURSE, "Monsters/Monster_Dude.anm")) return false;
	if (!LoadMeshAnimations(&monsterCrabAnim, MESHDIRCOURSE, "Monsters/Monster_Crab.anm")) return false;

	return true;
}

bool Renderer::InitSkybox()
{
	skybox = new Skybox();
	if (skybox == nullptr)
		return false;
	
	return true;
}

void Renderer::InitWater()
{
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(waterBump, true);

	waterPosition.x = terrainHeightmapSize.x * 0.5f;
	waterPosition.y = terrainHeightmapSize.y * 0.2355f;
	waterPosition.z = terrainHeightmapSize.z * 0.5f;
	waterRotate = 0.0f;
	waterCycle = 0.0f;	
}

void Renderer::InitData()
{
	LoadTreeData(ROCK2FILE, rock2Mesh, rockMaterial, rocks2ParentNode);
	LoadTreeData(TREESFILE, treeMesh, treeMaterial, treesParentNode, true);
	LoadTreeData(CASTLEFILE, castleMesh, castleMaterial, castleParentNode);
	LoadTreeData(CASTLEPILLARFILE, castlePillarMesh, castlePillarMaterial, castlePillarParentNode);
	LoadTreeData(CASTLEARCHFILE, castleArchMesh, castleArchMaterial, castleArchParentNode);
	LoadTreeData(CASTLEBRIDGEFILE, castleBridgeMesh, castleBridgeMaterial, castleBridgeParentNode);
	LoadTreeData(RUINSFILE, ruinsMesh, ruinsMaterial, ruinsParentNode);
	LoadTreeData(CRYSTAL01FILE, crystal1Mesh, crystal1Material, crystals1ParentNode);
	LoadTreeData(CRYSTAL02FILE, crystal2Mesh, crystal2Material, crystals2ParentNode);

	LoadAnimNodeData(MONSTERDUDEFILE, monsterDudeMesh, monsterDudeMaterial, monsterDudeAnim, monsterDudeParentNode, false);
	LoadAnimNodeData(MONSTERCRABFILE, monsterCrabMesh, monsterCrabMaterial, monsterCrabAnim, monsterCrabParentNode, false);
}

void Renderer::InitFog()
{
	if (!FileHandler::FileExists(FOGDATAFILE))
		return;
	
	FileHandler::ReadFogFile(FOGDATAFILE, enableFog, fogColour);
}

void Renderer::SetupShadows()
{
	//Shadows
	glEnable(GL_DEPTH_TEST);

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	//----------------------

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Renderer::LoadMesh(Mesh** mesh, const std::string& pathPrefix, const std::string& meshFileName)
{
	*mesh = Mesh::LoadFromMeshFile(pathPrefix + meshFileName);
	if (*mesh == nullptr)
		return false;

	return true;
}

bool Renderer::LoadShader(Shader** shader, const std::string& pathPrefix, const std::string& shaderVertexFileName, const std::string& shaderFragmentFileName)
{
	*shader = new Shader(pathPrefix + shaderVertexFileName, pathPrefix + shaderFragmentFileName);
	if (*shader == nullptr)
		return false;
	
	return true;
}

bool Renderer::LoadMeshMaterial(MeshMaterial** material, const std::string& pathPrefix, const std::string& materialFileName)
{
	*material = new MeshMaterial(pathPrefix + materialFileName, true);
	if (*material == nullptr)
		return false;

	return true;
}

bool Renderer::LoadMeshAnimations(MeshAnimation** anim, const std::string& pathPrefix, const std::string& animFileName)
{
	*anim = new MeshAnimation(pathPrefix + animFileName, true);
	if (*anim == nullptr)
		return false;

	return true;
}

//------------------------------------------------------------------

//------------------------------------------------------------------

void Renderer::NewTreeProp(Mesh* m, MeshMaterial* mMat, SceneNode* parent, bool isTransparent)
{
	TreePropNode* tree = new TreePropNode(m, mMat, basicDiffuseShader, TEXTUREDIR);
	if (isTransparent)
		tree->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	tree->SetModelScale(Vector3(6.0f, 6.0f, 6.0f));
	tree->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * tree->GetRotationMatrix() * Matrix4::Scale(tree->GetModelScale()));
	parent->AddChild(tree);
}

void Renderer::NewTreeProp(Mesh* m, MeshMaterial* mMat, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent, bool isTransparent)
{
	TreePropNode* tree = new TreePropNode(m, mMat, basicDiffuseShader, TEXTUREDIR);
	if (isTransparent)
		tree->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	tree->SetModelRotation(Rot);
	tree->SetModelScale(Scale);
	tree->SetTransform(Matrix4::Translation(Pos) * tree->GetRotationMatrix() * Matrix4::Scale(tree->GetModelScale()));
	parent->AddChild(tree);
}

void Renderer::LoadTreeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, SceneNode* parent, bool isTransparent)
{
	if (FileHandler::FileExists(fileName))
	{
		std::vector<Vector3> posV, rotV, scaleV;
		FileHandler::ReadPropDataFromFile(fileName, posV, rotV, scaleV);

		for (int i = 0; i < posV.size(); i++)
			NewTreeProp(m, mMat, posV[i], rotV[i], scaleV[i], parent, isTransparent);
	}
	else
		NewTreeProp(m, mMat, parent, isTransparent);
}

//------------------------------------------------------------------

void Renderer::NewAnimNodeProp(Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, SceneNode* parent, bool isTransparent)
{
	AnimMeshNode* anim = new AnimMeshNode(skeletalAnimShader, m, mAnim, mMat, TEXTUREDIR);
	if (isTransparent)
		anim->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	anim->SetModelScale(Vector3(6.0f, 6.0f, 6.0f));
	anim->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * anim->GetRotationMatrix() * Matrix4::Scale(anim->GetModelScale()));
	parent->AddChild(anim);
}

void Renderer::NewAnimNodeProp(Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, const Vector3& Pos, const Vector3& Rot, const Vector3& Scale, SceneNode* parent, bool isTransparent)
{
	AnimMeshNode* anim = new AnimMeshNode(skeletalAnimShader, m, mAnim, mMat, TEXTUREDIR);
	if (isTransparent)
		anim->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	anim->SetModelRotation(Rot);
	anim->SetModelScale(Scale);
	anim->SetTransform(Matrix4::Translation(Pos) * anim->GetRotationMatrix() * Matrix4::Scale(anim->GetModelScale()));
	parent->AddChild(anim);
}

void Renderer::LoadAnimNodeData(const std::string& fileName, Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, SceneNode* parent, bool isTransparent)
{
	if (FileHandler::FileExists(fileName))
	{
		std::vector<Vector3> posV, rotV, scaleV;
		FileHandler::ReadPropDataFromFile(fileName, posV, rotV, scaleV);

		for (int i = 0; i < posV.size(); i++)
			NewAnimNodeProp(m, mMat, mAnim, posV[i], rotV[i], scaleV[i], parent, isTransparent);
	}
	else
		NewAnimNodeProp(m, mMat, mAnim, parent, isTransparent);
}

//------------------------------------------------------------------

void Renderer::CreateNewPointLight()
{
	Light* l = new Light();
	l->SetPosition(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f));
	l->SetColour(Vector4(1.0f, 0.0f, 0.0f, 0.0f));
	l->SetRadius(terrainHeightmapSize.x * 0.2f);

	allPointLights.push_back(*l);
	numPointLights = (int)allPointLights.size();
}

//------------------------------------------------------------------

void Renderer::UpdateScene(float dt) 
{
	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;

	if (cameraPathManager != nullptr && cameraPathManager->GetMode() == 0)
		cameraMain->UpdateCamera(dt);

	//viewMatrix = cameraMain->BuildViewMatrix();

	if (cameraPathManager != nullptr)
		cameraPathManager->Update(dt, (float)timer->GetTotalTimeSeconds());

	rootNode->Update(dt);
	//dirLight->SetLightDir(Vector3(sin(timer->GetTotalTimeSeconds()), cos(timer->GetTotalTimeSeconds()), 0));
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
			NewTreeProp(rock2Mesh, rockMaterial, rocks2ParentNode);

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
	/*if (ImGui::CollapsingHeader("Rocks5A"))
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
	}*/
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
			NewTreeProp(treeMesh, treeMaterial, treesParentNode, true);

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
	if (ImGui::CollapsingHeader("Castle Pillar"))
	{
		for (int i = 0; i < castlePillarParentNode->GetChildCount() && (castlePillarParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = castlePillarParentNode->GetChild(i);
			ImGui::Text((std::string("Pillar[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("New Castle Pillar"))
			NewTreeProp(castlePillarMesh, castlePillarMaterial, castlePillarParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save CastlePillarProp.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < castlePillarParentNode->GetChildCount() && (castlePillarParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = castlePillarParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CASTLEPILLARFILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Castle Bridge"))
	{
		for (int i = 0; i < castleBridgeParentNode->GetChildCount() && (castleBridgeParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = castleBridgeParentNode->GetChild(i);
			ImGui::Text((std::string("Bridge[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("Save CastleBridge.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < castleBridgeParentNode->GetChildCount() && (castleBridgeParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = castleBridgeParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CASTLEBRIDGEFILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Castle Arch"))
	{
		for (int i = 0; i < castleArchParentNode->GetChildCount() && (castleArchParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = castleArchParentNode->GetChild(i);
			ImGui::Text((std::string("Arch[") + indexStr + "]:").c_str());

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

		if (ImGui::Button("New Castle Arch"))
			NewTreeProp(castleArchMesh, castleArchMaterial, castleArchParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save CastleArchProp.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < castleArchParentNode->GetChildCount() && (castleArchParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = castleArchParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CASTLEARCHFILE, nodePosV, nodeRotV, nodeScaleV);
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
	if (ImGui::CollapsingHeader("Crystals 01"))
	{
		for (int i = 0; i < crystals1ParentNode->GetChildCount() && (crystals1ParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = crystals1ParentNode->GetChild(i);
			ImGui::Text((std::string("Crys1[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("New Crystal 01"))
			NewTreeProp(crystal1Mesh, crystal1Material, crystals1ParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save Crystal01.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < crystals1ParentNode->GetChildCount() && (crystals1ParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = crystals1ParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CRYSTAL01FILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Crystals 02"))
	{
		for (int i = 0; i < crystals2ParentNode->GetChildCount() && (crystals2ParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = crystals2ParentNode->GetChild(i);
			ImGui::Text((std::string("Crys2[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("New Crystal 02"))
			NewTreeProp(crystal2Mesh, crystal2Material, crystals2ParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save Crystal02.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < crystals2ParentNode->GetChildCount() && (crystals2ParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = crystals2ParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(CRYSTAL02FILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Monster - Dude"))
	{
		for (int i = 0; i < monsterDudeParentNode->GetChildCount() && (monsterDudeParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = monsterDudeParentNode->GetChild(i);
			ImGui::Text((std::string("Dude[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("New Monster Dude"))
			NewAnimNodeProp(monsterDudeMesh, monsterDudeMaterial, monsterDudeAnim, monsterDudeParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save MonsterDude.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < monsterDudeParentNode->GetChildCount() && (monsterDudeParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = monsterDudeParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(MONSTERDUDEFILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Monster - Crabs"))
	{
		for (int i = 0; i < monsterCrabParentNode->GetChildCount() && (monsterCrabParentNode->GetChildCount() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			SceneNode* node = monsterCrabParentNode->GetChild(i);
			ImGui::Text((std::string("Crab[") + indexStr + "]:").c_str());

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
		if (ImGui::Button("New Monster Crab"))
			NewAnimNodeProp(monsterCrabMesh, monsterCrabMaterial, monsterCrabAnim, monsterCrabParentNode, false);

		ImGui::SameLine();
		if (ImGui::Button("Save MonsterCrab.sav"))
		{
			std::vector<Vector3> nodePosV, nodeRotV, nodeScaleV;
			for (int i = 0; i < monsterCrabParentNode->GetChildCount() && (monsterCrabParentNode->GetChildCount() > 0); i++)
			{
				SceneNode* node = monsterCrabParentNode->GetChild(i);
				nodePosV.emplace_back(node->GetWorldTransform().GetPositionVector());
				nodeRotV.emplace_back(node->GetModelRotation());
				nodeScaleV.emplace_back(node->GetModelScale());
			}
			FileHandler::SavePropDataToFile(MONSTERCRABFILE, nodePosV, nodeRotV, nodeScaleV);
		}
	}
	if (ImGui::CollapsingHeader("Lights"))
	{
		Vector3 lightDir = dirLight->GetLightDir();
		Vector4 lightDirColor = dirLight->GetColour();
		if (ImGui::SliderFloat3(_labelPrefix(std::string("Direction").c_str()).c_str(), (float*)&lightDir, 0.0f, 1.0f))
			dirLight->SetLightDir(lightDir);

		if (ImGui::SliderFloat4(_labelPrefix(std::string("Colour").c_str()).c_str(), (float*)&lightDirColor, 0.0f, 1.0f))
			dirLight->SetColour(lightDirColor);

		float lightIntensity = dirLight->GetIntensity();
		if (ImGui::SliderFloat(_labelPrefix(std::string("Intensity").c_str()).c_str(), &lightIntensity, 0.0f, 10.0f))
			dirLight->SetIntensity(lightIntensity);

		ImGui::Separator();

		for (int i = 0; i < allPointLights.size() && (allPointLights.size() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			Light& pLight = allPointLights[i];
			Vector3 pLightPos = pLight.GetPosition();
			Vector4 pLightColour = pLight.GetColour();
			Vector4 pLightSpecColour = pLight.GetSpecularColour();
			float pLightRadius = pLight.GetRadius();
			float pLightIntensity = pLight.GetIntensity();

			if (ImGui::DragFloat3(_labelPrefix(std::string("Light Pos[" + indexStr + "]").c_str()).c_str(), (float*)&pLightPos))
				pLight.SetPosition(pLightPos);

			if (ImGui::SliderFloat4(_labelPrefix(std::string("Light Col[" + indexStr + "]").c_str()).c_str(), (float*)&pLightColour, 0.0f, 1.0f))
				pLight.SetColour(pLightColour);

			if (ImGui::SliderFloat4(_labelPrefix(std::string("Light Spec Col[" + indexStr + "]").c_str()).c_str(), (float*)&pLightSpecColour, 0.0f, 1.0f))
				pLight.SetSpecularColour(pLightSpecColour);

			if(ImGui::DragFloat(_labelPrefix(std::string("Light Radius[" + indexStr + "]").c_str()).c_str(), &pLightRadius))
				pLight.SetRadius(pLightRadius);

			if (ImGui::DragFloat(_labelPrefix(std::string("Light Intensity[" + indexStr + "]").c_str()).c_str(), &pLightIntensity))
				pLight.SetIntensity(pLightIntensity);

			ImGui::Separator();
		}

		if (ImGui::Button("New Point Light"))
			CreateNewPointLight();

		ImGui::SameLine();
		if (ImGui::Button("Save LightsData.sav"))
			FileHandler::SaveLightDataFile(LIGHTSDATAFILE, *dirLight, allPointLights);
	}
	if (ImGui::CollapsingHeader("Fog"))
	{
		ImGui::Checkbox(_labelPrefix(std::string("Enable Fog").c_str()).c_str(), &enableFog);
		ImGui::SliderFloat4(_labelPrefix(std::string("Fog Colour").c_str()).c_str(), (float*)&fogColour, 0.0f, 1.0f);

		if (ImGui::Button("Save FogData.sav"))
			FileHandler::SaveFogFile(FOGDATAFILE, enableFog, fogColour);
	}
	if (ImGui::CollapsingHeader("Water"))
		ImGui::DragFloat3(_labelPrefix(std::string("Water Position").c_str()).c_str(), (float*)&waterPosition);
	if (ImGui::CollapsingHeader("Camera Paths Manager") && (cameraPathManager != nullptr))
	{
		for (size_t i = 0; i < cameraPathManager->GetPathsDataSize() && (cameraPathManager->GetPathsDataSize() > 0); i++)
		{
			std::string indexStr = std::to_string(i);
			ImGui::DragFloat3(_labelPrefix(std::string("PathPos[" + indexStr + "]").c_str()).c_str(), (float*)&cameraPathManager->GetPathPos((int)i));
			ImGui::DragFloat3(_labelPrefix(std::string("PathRot[" + indexStr + "]").c_str()).c_str(), (float*)&cameraPathManager->GetPathRot((int)i));
			
			float delay = cameraPathManager->GetPathDelay((int)i);
			if (ImGui::DragFloat(_labelPrefix(std::string("Delay[" + indexStr + "]").c_str()).c_str(), &delay))
				cameraPathManager->SetPathDelay((int)i, delay);

			ImGui::Separator();
		}

		if (ImGui::Button("Save Data"))
			cameraPathManager->Save();
		ImGui::SameLine();
		if (ImGui::Button("Add Cam Data"))
			cameraPathManager->AddPathData(cameraMain->getPosition(), cameraMain->getRotation());

		if (ImGui::Button("Play"))
			cameraPathManager->SetMode(1);
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
			cameraPathManager->SetMode(0);

		float s = cameraPathManager->GetSpeed();
		if (ImGui::DragFloat(_labelPrefix(std::string("PathSpeed").c_str()).c_str(), &s))
			cameraPathManager->SetSpeed(s);
	}

	//ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::RenderScene()	
{
	if (blendFix)
	{
		BuildNodeLists(rootNode);
		SortNodeLists();
	}

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawShadowScene();
	DrawSkybox();
	if (blendFix)
	{
		DrawNodes();
		ClearNodeLists();
	}
	else
		DrawNode(rootNode);	

	UpdateImGui();
}

void Renderer::DrawSkybox()
{
	if (skybox == nullptr)
		return;

	glDepthMask(GL_FALSE);

	BindShader(skybox->GetSkyboxShader());
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawShadowScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	
	//viewMatrix = Matrix4::BuildViewMatrix(light->GetDirection(), Vector3(heightMap->GetHeightmapSize().x / 2, 0, heightMap->GetHeightmapSize().z / 2));
	float _nearPlane = 1.0f, _farPlane = 100.0f;
	//projMatrix = Matrix4::Orthographic(-100.0f, 100.0f, -100.0f, 100.0f, _nearPlane, _farPlane);
	projMatrix = Matrix4::Perspective(1, 100.0f, 1, 60.0f);
	viewMatrix = Matrix4::BuildViewMatrix(allPointLights[numPointLights - 1].GetPosition(), Vector3(0, 0, 0));
	shadowMatrix = projMatrix * viewMatrix;

	UpdateShaderMatrices();

	DrawNode(rootNode, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Renderer::BuildNodeLists(SceneNode* from)
{
	Vector3 dir = from->GetWorldTransform().GetPositionVector() - cameraMain->getPosition();
	from->SetCameraDistance(Vector3::Dot(dir, dir));

	if (from->GetColour().w < 1.0f)
		transparentNodesList.push_back(from);
	else
		opaqueNodesList.push_back(from);

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i)
		BuildNodeLists((*i));
}

void Renderer::SortNodeLists()
{
	std::sort(transparentNodesList.rbegin(), transparentNodesList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(opaqueNodesList.begin(), opaqueNodesList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes()
{
	for (const auto& i : opaqueNodesList)
		DrawNode(i, false);
	DrawWater();
	for (const auto& i : transparentNodesList)
		DrawNode(i, false);
}

void Renderer::DrawNode(SceneNode* n, bool includingChild)
{
	if (n->GetMesh())
	{
		BindShader(n->GetShader());
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());

		viewMatrix = cameraMain->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

		UpdateShaderMatrices();

		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "cameraPos"), 1, (float*)&cameraMain->getPosition());

		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDir"), 1, (float*)&dirLight->GetLightDir());
		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirColour"), 1, (float*)&dirLight->GetColour());
		glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirIntensity"), dirLight->GetIntensity());

		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "enableFog"), enableFog);
		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "fogColour"), 1, (float*)&fogColour);

		SetShaderLight(allPointLights[numPointLights - 1]);		//Sun

		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "numPointLights"), numPointLights);
		for (size_t i = 0; i < allPointLights.size() && (numPointLights > 0); i++)
		{
			Light& l = allPointLights[i];

			std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
			std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
			std::string lightSpecularColourName = "pointLightSpecularColour[" + std::to_string(i) + "]";
			std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
			std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";

			glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), lightPosName.c_str()), 1, (float*)&l.GetPosition());
			glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), lightColorName.c_str()), 1, (float*)&l.GetColour());
			glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), lightSpecularColourName.c_str()), 1, (float*)&l.GetSpecularColour());
			glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), lightRadiusName.c_str()), l.GetRadius());
			glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), lightIntensityName.c_str()), l.GetIntensity());
		}

		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowTex"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

		n->Draw(*this);
		
		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
	}
	
	if (includingChild)
	{
		for (std::vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
			DrawNode(*i);
	}
}

void Renderer::ClearNodeLists()
{
	transparentNodesList.clear();
	opaqueNodesList.clear();
	opaqueNodesList.push_back(terrainNode);
}

void Renderer::DrawWater()
{
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&cameraMain->getPosition());
	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "lightDir"), 1, (float*)&dirLight->GetLightDir());
	glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), "lightDirColour"), 1, (float*)&dirLight->GetColour());
	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "lightDirIntensity"), dirLight->GetIntensity());
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "enableFog"), enableFog);
	glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), "fogColour"), 1, (float*)&fogColour);
	SetShaderLight(*dirLight);		//Sun

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "numPointLights"), numPointLights);
	for (size_t i = 0; i < allPointLights.size() && (numPointLights > 0); i++)
	{
		Light& l = allPointLights[i];

		std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
		std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
		std::string lightSpecularColourName = "pointLightSpecularColour[" + std::to_string(i) + "]";
		std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
		std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";

		glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), lightPosName.c_str()), 1, (float*)&l.GetPosition());
		glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), lightColorName.c_str()), 1, (float*)&l.GetColour());
		glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), lightSpecularColourName.c_str()), 1, (float*)&l.GetSpecularColour());
		glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), lightRadiusName.c_str()), l.GetRadius());
		glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), lightIntensityName.c_str()), l.GetIntensity());
	}

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBump);

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetSkyboxCube());

	modelMatrix = Matrix4::Translation(waterPosition) * Matrix4::Scale(terrainHeightmapSize * 0.5f) * Matrix4::Rotation(-90.0f, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	quad->Draw();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}