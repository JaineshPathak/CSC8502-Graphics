#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Light.h"
#include "../nclgl/DirectionalLight.h"
#include <algorithm>

#define SHADOW_SIZE 256

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
	cube = Mesh::GenerateCube();
	depthQuadShader = new Shader(SHADERDIRCOURSETERRAIN"DebugQuadVert2.glsl", SHADERDIRCOURSETERRAIN"DebugQuadFrag2.glsl");
	unlitDiffuseShader = new Shader(SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");

	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");

	//std::cout << "V4 Size: " << sizeof(Vector4) << std::endl;
	//std::cout << "M4 Size: " << sizeof(Matrix4) << std::endl;

	//-----------------------------------------------------------
	//Imgui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
	//-----------------------------------------------------------

	boundingRadiusMultiplier = 8.5f;

	rootNode = new SceneNode();
	rootNode->nodeName = "Root";

	terrainNode = new TerrainNode();
	
	//------------------------------------------------------------------
	//Rocks
	rocks2ParentNode = new SceneNode();
	rocks2ParentNode->nodeName = "Rocks2Parent";
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

	castlePillarParentNode = new SceneNode();
	castlePillarParentNode->nodeName = "CastlePillarsParent";

	castleArchParentNode = new SceneNode();
	castleArchParentNode->nodeName = "CastleArchParent";
	
	castleBridgeParentNode = new SceneNode();
	castleBridgeParentNode->nodeName = "CastleBridgeParent";
	//------------------------------------------------------------------

	//Ruins
	ruinsParentNode = new SceneNode();
	ruinsParentNode->nodeName = "RuinsParent";
	//------------------------------------------------------------------
	
	// Crystals
	crystals1ParentNode = new SceneNode();
	crystals1ParentNode->nodeName = "Crystal01Parent";
	crystals2ParentNode = new SceneNode();
	crystals2ParentNode->nodeName = "Crystal02Parent";
	//------------------------------------------------------------------

	//Monsters
	monsterDudeParentNode = new SceneNode();
	monsterDudeParentNode->nodeName = "MonstersDudeParent";

	monsterCrabParentNode = new SceneNode();
	monsterCrabParentNode->nodeName = "MonstersCrabParent";
	//------------------------------------------------------------------

	
	rootNode->AddChild(terrainNode);
	terrainNode->AddChild(rocks2ParentNode);
	//terrainNode->AddChild(rocks5aParentNode);
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

	terrainHeightmapSize = terrainNode->GetHeightmapSize();
	cameraMain = new Camera();
	cameraMain->SetDefaultSpeed(850.0f);
	cameraMain->SetPosition(terrainHeightmapSize * Vector3(0.5f, 2.5f, 0.5f));

	cameraPathManager = new CameraPathsManager(enableAutoCameraPaths, CAMERAPATHS, cameraMain);

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

	basicDiffuseShader = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragmentv2.glsl");
	basicDiffuseShaderInstanced = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedVertexv3.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragmentv2.glsl");
	
	//Rocks
	rock2Amount = 0;
	rock2Mesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Rocks/Mesh_Rock5D.msh");
	rockMaterial = new MeshMaterial(MESHDIRCOURSE"Rocks/Mesh_Rock5D.mat", true);
	SetupMeshTextures(rock2Mesh, rockMaterial, rockMatTextures, rockMatBumpTextures);
	
	//LoadTreeData(ROCK2FILE, rock2Mesh, rockMaterial, rocks2ParentNode);
	LoadPropData(ROCK2FILE, rock2Mesh, rock2Amount);
	
	//Trees
	treeMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Trees/Tree_01.msh");
	treeMaterial = new MeshMaterial(MESHDIRCOURSE"Trees/Tree_01.mat", true);
	LoadTreeData(TREESFILE, treeMesh, treeMaterial, treesParentNode, true);

	//Castle
	castleMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Castle/Mesh_CastleMain.msh");
	castleMaterial = new MeshMaterial(MESHDIRCOURSE"Castle/Mesh_CastleMain.mat", true);
	LoadTreeData(CASTLEFILE, castleMesh, castleMaterial, castleParentNode);

	//Castle Pillar
	castlePillarAmount = 0;
	castlePillarMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Castle/Mesh_CastlePillar.msh");
	castlePillarMaterial = new MeshMaterial(MESHDIRCOURSE"Castle/Mesh_CastlePillar.mat", true);
	SetupMeshTextures(castlePillarMesh, castlePillarMaterial, castlePillarMatTextures, castlePillarMatBumpTextures);
	LoadPropData(CASTLEPILLARFILE, castlePillarMesh, castlePillarAmount, 6.0f);
	//LoadTreeData(CASTLEPILLARFILE, castlePillarMesh, castlePillarMaterial, castlePillarParentNode);

	//Castle Arch
	castleArchAmount = 0;
	castleArchMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Castle/Mesh_CastleArch.msh");
	castleArchMaterial = new MeshMaterial(MESHDIRCOURSE"Castle/Mesh_CastleArch.mat", true);
	SetupMeshTextures(castleArchMesh, castleArchMaterial, castleArchMatTextures, castleArchMatBumpTextures);
	//LoadTreeData(CASTLEARCHFILE, castleArchMesh, castleArchMaterial, castleArchParentNode);
	LoadPropData(CASTLEARCHFILE, castleArchMesh, castleArchAmount, 10.0f);

	//Castle Bridge
	castleBridgeMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Castle/Mesh_Bridge.msh");
	castleBridgeMaterial = new MeshMaterial(MESHDIRCOURSE"Castle/Mesh_Bridge.mat", true);
	LoadTreeData(CASTLEBRIDGEFILE, castleBridgeMesh, castleBridgeMaterial, castleBridgeParentNode);

	//Ruins
	ruinsMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Ruins/Mesh_RuinsMain.msh");
	ruinsMaterial = new MeshMaterial(MESHDIRCOURSE"Ruins/Mesh_RuinsMain.mat", true);
	LoadTreeData(RUINSFILE, ruinsMesh, ruinsMaterial, ruinsParentNode);

	//Crystals
	crystal1Amount = 0;
	crystal2Amount = 0;

	crystal1Mesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Crystals/Mesh_Crystal_01.msh");
	crystal2Mesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Crystals/Mesh_Crystal_02.msh");

	crystal1Material = new MeshMaterial(MESHDIRCOURSE"Crystals/Mesh_Crystal_01.mat", true);
	crystal2Material = new MeshMaterial(MESHDIRCOURSE"Crystals/Mesh_Crystal_02.mat", true);

	SetupMeshTextures(crystal1Mesh, crystal1Material, crystal1MatTextures, crystal1MatBumpTextures);
	SetupMeshTextures(crystal2Mesh, crystal2Material, crystal2MatTextures, crystal2MatBumpTextures);

	LoadPropData(CRYSTAL01FILE, crystal1Mesh, crystal1Amount, 12.0f);
	LoadPropData(CRYSTAL02FILE, crystal2Mesh, crystal2Amount, 12.0f);

	//LoadTreeData(CRYSTAL01FILE, crystal1Mesh, crystal1Material, crystals1ParentNode);
	//LoadTreeData(CRYSTAL02FILE, crystal2Mesh, crystal2Material, crystals2ParentNode);

	//Monsters
	//Ghoul
	skeletalAnimShader = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedSkinVertex.glsl", SHADERDIRCOURSETERRAIN"CWTexturedSkinFragment.glsl");

	//Dude
	monsterDudeMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Monsters/Monster_Dude.msh");
	monsterDudeMaterial = new MeshMaterial(MESHDIRCOURSE"Monsters/Monster_Dude.mat", true);
	monsterDudeAnim = new MeshAnimation(MESHDIRCOURSE"Monsters/Monster_Dude.anm", true);
	LoadAnimNodeData(MONSTERDUDEFILE, monsterDudeMesh, monsterDudeMaterial, monsterDudeAnim, monsterDudeParentNode, false);

	//Crab
	monsterCrabMesh = Mesh::LoadFromMeshFile(MESHDIRCOURSE"Monsters/Monster_Crab.msh");
	monsterCrabMaterial = new MeshMaterial(MESHDIRCOURSE"Monsters/Monster_Crab.mat", true);
	monsterCrabAnim = new MeshAnimation(MESHDIRCOURSE"Monsters/Monster_Crab.anm", true);
	LoadAnimNodeData(MONSTERCRABFILE, monsterCrabMesh, monsterCrabMaterial, monsterCrabAnim, monsterCrabParentNode, false);
	
	//Lights
	dirLight = new DirectionalLight(Vector3(1, 1, 0), Vector4(1.0f, 0.8703716f, 0.7294118f, 1.0f), Vector4());
	dirLight->SetPosition(Vector3(2048.0f, 1628.0f, 2048.0f));
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
	currentLightIndex = 1;
	lightLookAtPos = terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f);

	//Skybox Cubemap
	skybox = new Skybox();

	//Water
	reflectShader = new Shader(SHADERDIRCOURSETERRAIN"CWReflectVertex.glsl", SHADERDIRCOURSETERRAIN"CWReflectFragment.glsl");
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBump = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(waterBump, true);
	waterPosition.x = terrainHeightmapSize.x * 0.5f;
	waterPosition.y = terrainHeightmapSize.y * 0.2355f;
	waterPosition.z = terrainHeightmapSize.z * 0.5f;
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	
	//Fog
	if (FileHandler::FileExists(FOGDATAFILE))
		FileHandler::ReadFogFile(FOGDATAFILE, enableFog, fogColour);

	//Shadows

	/*shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);*/

	//glBindTexture(GL_TEXTURE_2D, 0);

	//----------------------

	/*glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	//----------------------	

	/*glGenFramebuffers(1, &sampleFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sampleFBO);

	glGenTextures(1, &sampleFBOTex);
	glBindTexture(GL_TEXTURE_2D, sampleFBOTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sampleFBOTex, 0);

	glGenRenderbuffers(1, &sampleRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, sampleRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sampleRBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
	//----------------------

	//projMatrix = Matrix4::Perspective(1.0, 10000.0f, (float)width / (float)height, 60.0f);	

	//---------------------------------------------------------------------------------------------------
	//POST PROCESSING!!!!!!!!!!!!!!!...................................
	
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; i++)
	{
		glGenTextures(1, &bufferColorTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColorTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &bufferPostProcessFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColorTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColorTex[0])
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//---------------------------------------------------------------------------------------------------
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

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

	delete basicDiffuseShader;
	delete skeletalAnimShader;
	delete reflectShader;
	//delete shadowShader;

	//glDeleteTextures(1, &shadowTex);
	//glDeleteFramebuffers(1, &shadowFBO);

	delete timer;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//------------------------------------------------------------------

//------------------------------------------------------------------

void Renderer::SetupMeshTextures(Mesh* mesh, MeshMaterial* meshMaterial, std::vector<GLuint>& matTexturesV, std::vector<GLuint>& matTexturesBumpV)
{
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		const MeshMaterialEntry* matEntry = meshMaterial->GetMaterialForLayer(i);

		const string* fileName = nullptr;
		matEntry->GetEntry("Diffuse", &fileName);
		string path = TEXTUREDIR + *fileName;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		OGLRenderer::SetTextureRepeating(texID, true);
		matTexturesV.emplace_back(texID);

		const string* bumpFileName = nullptr;
		matEntry->GetEntry("Bump", &bumpFileName);
		string bumpPath = TEXTUREDIR + *bumpFileName;
		GLuint bumptexID = SOIL_load_OGL_texture(bumpPath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		OGLRenderer::SetTextureRepeating(bumptexID, true);
		matTexturesBumpV.emplace_back(bumptexID);
	}
}

void Renderer::NewTreeProp(Mesh* m, MeshMaterial* mMat, SceneNode* parent, bool isTransparent)
{
	TreePropNode* tree = new TreePropNode(m, mMat, basicDiffuseShader, TEXTUREDIR);
	if (isTransparent)
		tree->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	tree->SetModelScale(Vector3(6.0f, 6.0f, 6.0f));
	tree->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * tree->GetRotationMatrix() * Matrix4::Scale(tree->GetModelScale()));
	tree->SetBoundingRadius(tree->GetModelScale().GetAbsMaxElement() * boundingRadiusMultiplier);
	tree->CalcBoundingBox();
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
	tree->SetBoundingRadius(tree->GetModelScale().GetAbsMaxElement() * boundingRadiusMultiplier);
	tree->CalcBoundingBox();
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

void Renderer::LoadPropData(const std::string& propFilename, Mesh* m, int& propAmount, const float& extraScale)
{
	if (FileHandler::FileExists(propFilename))
	{
		std::vector<Vector3> posV, rotV, scaleV;
		FileHandler::ReadPropDataFromFile(propFilename, posV, rotV, scaleV);

		if ((int)posV.size() > 0)
		{
			propAmount = (int)posV.size();

			Matrix4* modelMatrices = new Matrix4[(int)posV.size()];
			for (int i = 0; i < propAmount; i++)
			{	
				Matrix4 model, modelRotMat;
				
				modelRotMat = Matrix4::Rotation(rotV[i].x, Vector3(1, 0, 0)) * Matrix4::Rotation(rotV[i].y, Vector3(0, 1, 0)) * Matrix4::Rotation(rotV[i].z, Vector3(0, 0, 1));

				model = Matrix4::Translation(posV[i]) * modelRotMat * Matrix4::Scale(scaleV[i] * extraScale);
				modelMatrices[i] = model;
			}

			//VBO
			unsigned int VBO;
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, propAmount * sizeof(Matrix4), &modelMatrices[0], GL_STATIC_DRAW);

			glBindVertexArray(m->GetArrayObject());
			// set attribute pointers for matrix (4 times vec4)
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void*)0);
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void*)(sizeof(Vector4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void*)(2 * sizeof(Vector4)));
			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void*)(3 * sizeof(Vector4)));

			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
			glVertexAttribDivisor(9, 1);

			glBindVertexArray(0);
		}
	}
}

//------------------------------------------------------------------

void Renderer::NewAnimNodeProp(Mesh* m, MeshMaterial* mMat, MeshAnimation* mAnim, SceneNode* parent, bool isTransparent)
{
	AnimMeshNode* anim = new AnimMeshNode(skeletalAnimShader, m, mAnim, mMat, TEXTUREDIR);
	if (isTransparent)
		anim->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	anim->SetModelScale(Vector3(6.0f, 6.0f, 6.0f));
	anim->SetTransform(Matrix4::Translation(terrainHeightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * anim->GetRotationMatrix() * Matrix4::Scale(anim->GetModelScale()));
	anim->SetBoundingRadius(anim->GetModelScale().GetAbsMaxElement() * boundingRadiusMultiplier);
	anim->CalcBoundingBox();
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
	anim->SetBoundingRadius(anim->GetModelScale().GetAbsMaxElement() * boundingRadiusMultiplier);
	anim->CalcBoundingBox();
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

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

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
		int lightIndex = currentLightIndex;
		if (ImGui::DragInt(_labelPrefix(std::string("Selected").c_str()).c_str(), &lightIndex, 1.0f, 0, numPointLights - 1))
			currentLightIndex = lightIndex;

		ImGui::Separator();

		Vector3 lightDirPos = dirLight->GetPosition();
		Vector3 lightDir = dirLight->GetLightDir();
		Vector4 lightDirColor = dirLight->GetColour();
		if (ImGui::DragFloat3(_labelPrefix(std::string("Postion").c_str()).c_str(), (float*)&lightDirPos, 1.0f, 0.0f, terrainHeightmapSize.x))
			dirLight->SetPosition(lightDirPos);

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
	if (ImGui::CollapsingHeader("Shadows"))
	{
		float curZNear = zNear;
		float curZFar = zFar;
		float curZLeft = zLeft;
		float curZRight = zRight;
		float curZTop = zTop;
		float curZBottom = zBottom;

		if (ImGui::DragFloat(_labelPrefix(std::string("zNear").c_str()).c_str(), &curZNear, 1.0f, -25.0f, 25.0f))
			zNear = curZNear;
		if (ImGui::DragFloat(_labelPrefix(std::string("zFar").c_str()).c_str(), &curZFar, 1.0f, -25.0f, 25.0f))
			zFar = curZFar;

		if (ImGui::DragFloat(_labelPrefix(std::string("zLeft").c_str()).c_str(), &curZLeft, 1.0f, -25.0f, 25.0f))
			zLeft = curZLeft;
		if (ImGui::DragFloat(_labelPrefix(std::string("zRight").c_str()).c_str(), &curZRight, 1.0f, -25.0f, 25.0f))
			zRight = curZRight;

		if (ImGui::DragFloat(_labelPrefix(std::string("zTop").c_str()).c_str(), &curZTop, 1.0f, -25.0f, 25.0f))
			zTop = curZTop;
		if (ImGui::DragFloat(_labelPrefix(std::string("zBottom").c_str()).c_str(), &curZBottom, 1.0f, -25.0f, 25.0f))
			zBottom = curZBottom;

		Vector3 currentLookAtPos = lightLookAtPos;
		if (ImGui::DragFloat3(_labelPrefix(std::string("Look At Pos").c_str()).c_str(), (float*)&currentLookAtPos))
			lightLookAtPos = currentLookAtPos;
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
	if (ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::DragInt(_labelPrefix(std::string("Blur Amount").c_str()).c_str(), (int*)&blurAmount, 1.0f, 0, 100);
	}

	//ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::RenderScene()
{
	BuildNodeLists(rootNode);
	SortNodeLists();

	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//DrawShadowScene();

	//DrawNodes();
	DrawSkybox();
	DrawAllInstances();
	PostProcessStage();
	FinalRender();
	//BloomBlurStage();

	ClearNodeLists();

	UpdateImGui();
}

void Renderer::DrawSkybox()
{
	if (skybox == nullptr)
		return;

	//glBindFramebuffer(GL_FRAMEBUFFER, sampleFBO);
	//glDisable(GL_DEPTH_TEST);
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	glDepthMask(GL_FALSE);

	BindShader(skybox->GetSkyboxShader());
	
	quad->Draw();

	//glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawShadowScene()
{
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT);

	BindShader(shadowShader);
	
	//float _nearPlane = -25.0f, _farPlane = 25.0f;
	//projMatrix = Matrix4::Orthographic(-100.0f, 100.0f, -100.0f, 100.0f, _nearPlane, _farPlane);
	//viewMatrix = Matrix4::BuildViewMatrix(dirLight->GetPosition(), dirLight->GetLightDir(), Vector3(0, 1, 0));
	
	//projMatrix = Matrix4::Perspective(1, 100.0f, 1, 45.0f);
	//viewMatrix = Matrix4::BuildViewMatrix(allPointLights[numPointLights - 1].GetPosition(), Vector3(0, 0, 0));
	Matrix4 lightProjMatrix = Matrix4::Orthographic(zNear, zFar, zRight, zLeft, zTop, zBottom);
	Matrix4 lightViewMatrix = Matrix4::BuildViewMatrix(dirLight->GetPosition(), lightLookAtPos, Vector3(0, 1, 0));

	lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
	//Matrix4 shadowProjMatrix = Matrix4::Perspective(1.0f, 100.0f, 1, 45.0f);
	//Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(allPointLights[currentLightIndex].GetPosition(), Vector3(0, 0, 0), Vector3(0, 1, 0));
	
	//shadowMatrix = shadowProjMatrix * shadowViewMatrix;
	DrawInstancedMesh(rock2Mesh, basicDiffuseShaderInstanced, rockMatTextures, rockMatBumpTextures, rock2Amount);
	DrawInstancedMesh(crystal1Mesh, basicDiffuseShaderInstanced, crystal1MatTextures, crystal1MatBumpTextures, crystal1Amount);
	DrawInstancedMesh(crystal2Mesh, basicDiffuseShaderInstanced, crystal2MatTextures, crystal2MatBumpTextures, crystal2Amount);
	DrawInstancedMesh(castleArchMesh, basicDiffuseShaderInstanced, castleArchMatTextures, castleArchMatBumpTextures, castleArchAmount);
	DrawInstancedMesh(castlePillarMesh, basicDiffuseShaderInstanced, castlePillarMatTextures, castlePillarMatBumpTextures, castlePillarAmount);

	for (const auto& i : opaqueNodesList)
	{
		//Matrix4 modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
		//Matrix4 mvpMatrix = shadowProjMatrix * shadowViewMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(shadowShader->GetProgram(), "lightSpaceMatrix"), 1, false, (float*)&lightSpaceMatrix);
		i->Draw(*this);
	}

	for (const auto& i : transparentNodesList)
	{
		//Matrix4 modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
		//Matrix4 mvpMatrix = shadowProjMatrix * shadowViewMatrix * modelMatrix;

		glUniformMatrix4fv(glGetUniformLocation(shadowShader->GetProgram(), "lightSpaceMatrix"), 1, false, (float*)&lightSpaceMatrix);
		i->Draw(*this);
	}

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);

	//Render Quad
	/*glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	BindShader(depthQuadShader);
	
	glUniform1i(glGetUniformLocation(depthQuadShader->GetProgram(), "shadowTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	quad->Draw();*/
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
	if (frameFrustum.InsideFrustumBox(*from))
	{
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - cameraMain->getPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f)
			transparentNodesList.push_back(from);
		else
			opaqueNodesList.push_back(from);

		display++;
	}
	total++;

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
	//std::cout << "Total process in CPU : " << total;
	//std::cout << " / Total send to GPU : " << display << std::endl;

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

		/*glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowTex"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);*/

		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightSpaceMatrix"), 1, false, (float*)&lightSpaceMatrix);

		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());

		viewMatrix = cameraMain->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

		UpdateShaderMatrices();

		//Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowMatrix"), 1, false, (float*)&fullShadowMat);

		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "cameraPos"), 1, (float*)&cameraMain->getPosition());

		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDir"), 1, (float*)&dirLight->GetLightDir());
		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirColour"), 1, (float*)&dirLight->GetColour());
		glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirIntensity"), dirLight->GetIntensity());

		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "enableFog"), enableFog);
		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "fogColour"), 1, (float*)&fogColour);

		//SetShaderLight(allPointLights[numPointLights - 1]);		//Sun
		SetShaderLight(*dirLight);

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

	total = 0;
	display = 0;
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

	OGLRenderer::BindTexture(waterTex, 0, "diffuseTex", reflectShader);
	OGLRenderer::BindTexture(waterBump, 1, "bumpTex", reflectShader);

	/*glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBump);*/

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

void Renderer::DrawAllInstances()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glEnable(GL_DEPTH_TEST);

	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(basicDiffuseShaderInstanced);

	/*glUniform1i(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "shadowTex"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, shadowTex);*/

	glUniformMatrix4fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "lightSpaceMatrix"), 1, false, (float*)&lightSpaceMatrix);

	viewMatrix = cameraMain->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

	UpdateShaderMatrices();

	glUniform3fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "cameraPos"), 1, (float*)&cameraMain->getPosition());

	glUniform3fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "lightDir"), 1, (float*)&dirLight->GetLightDir());
	glUniform4fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "lightDirColour"), 1, (float*)&dirLight->GetColour());
	glUniform1f(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "lightDirIntensity"), dirLight->GetIntensity());

	glUniform1i(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "enableFog"), enableFog);
	glUniform4fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "fogColour"), 1, (float*)&fogColour);

	//SetShaderLight(allPointLights[numPointLights - 1]);		//Sun
	SetShaderLight(*dirLight);

	glUniform1i(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), "numPointLights"), numPointLights);
	for (size_t i = 0; i < allPointLights.size() && (numPointLights > 0); i++)
	{
		Light& l = allPointLights[i];

		std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
		std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
		std::string lightSpecularColourName = "pointLightSpecularColour[" + std::to_string(i) + "]";
		std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
		std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";

		glUniform3fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), lightPosName.c_str()), 1, (float*)&l.GetPosition());
		glUniform4fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), lightColorName.c_str()), 1, (float*)&l.GetColour());
		glUniform4fv(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), lightSpecularColourName.c_str()), 1, (float*)&l.GetSpecularColour());
		glUniform1f(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), lightRadiusName.c_str()), l.GetRadius());
		glUniform1f(glGetUniformLocation(basicDiffuseShaderInstanced->GetProgram(), lightIntensityName.c_str()), l.GetIntensity());
	}

	//DrawInstancedMesh(rock2Mesh, rock2Tex, 0, "diffuseTex", basicDiffuseShaderInstanced, rock2Amount);
	DrawInstancedMesh(rock2Mesh, basicDiffuseShaderInstanced, rockMatTextures, rockMatBumpTextures, rock2Amount);
	DrawInstancedMesh(crystal1Mesh, basicDiffuseShaderInstanced, crystal1MatTextures, crystal1MatBumpTextures, crystal1Amount);
	DrawInstancedMesh(crystal2Mesh, basicDiffuseShaderInstanced, crystal2MatTextures, crystal2MatBumpTextures, crystal2Amount);
	DrawInstancedMesh(castleArchMesh, basicDiffuseShaderInstanced, castleArchMatTextures, castleArchMatBumpTextures, castleArchAmount);
	DrawInstancedMesh(castlePillarMesh, basicDiffuseShaderInstanced, castlePillarMatTextures, castlePillarMatBumpTextures, castlePillarAmount);
	
	DrawNodes();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDisable(GL_DEPTH_TEST);

	//// clear all relevant buffers
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//BindShader(depthQuadShader);
	//OGLRenderer::BindTexture(sampleFBOTex, 0, "screenTex", depthQuadShader);

	//UpdateShaderMatrices();

	//quad->Draw();

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

#pragma region REMOVED CUBE LIGHT TEST
	//-------------------------------------------------------------------------------
//LIGHT CUBE

/*glDisable(GL_CULL_FACE);
BindShader(unlitDiffuseShader);

modelMatrix = Matrix4::Translation(dirLight->GetPosition()) * Matrix4::Scale(Vector3(50, 50, 50));
viewMatrix = cameraMain->BuildViewMatrix();
projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

UpdateShaderMatrices();

glUniform1i(glGetUniformLocation(unlitDiffuseShader->GetProgram(), "useTexture"), 0);
glUniform4fv(glGetUniformLocation(unlitDiffuseShader->GetProgram(), "nodeColour"), 1, (float*)&Vector4(1.0f, 1.0f, 1.0f, 1.0f));

cube->Draw();

modelMatrix = Matrix4::Translation(lightLookAtPos) * Matrix4::Scale(Vector3(50, 50, 50));
viewMatrix = cameraMain->BuildViewMatrix();
projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

UpdateShaderMatrices();

glUniform1i(glGetUniformLocation(unlitDiffuseShader->GetProgram(), "useTexture"), 0);
glUniform4fv(glGetUniformLocation(unlitDiffuseShader->GetProgram(), "nodeColour"), 1, (float*)&Vector4(1.0f, 0.0f, 0.0f, 1.0f));

cube->Draw();
glEnable(GL_CULL_FACE);*/

//-------------------------------------------------------------------------------  
#pragma endregion
}

void Renderer::PostProcessStage()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferPostProcessFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColorTex[1], 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);

	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);

	bool vertical = false;
	for (int i = 0; i < blurAmount; ++i)
	{
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColorTex[1], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColorTex[0]);
		quad->Draw();
		
		// Now to swap the colour buffers , and do the second blur pass
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColorTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColorTex[1]);
		quad->Draw();

		//quad->Draw();

		vertical = !vertical;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Renderer::FinalRender()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(sceneShader);

	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColorTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex0"), 0);
	quad->Draw();
}

void Renderer::DrawInstancedMesh(Mesh* m, GLuint texID, GLuint unit, const std::string& uniformName, Shader* s, int amount)
{
	OGLRenderer::BindTexture(texID, unit, uniformName, s);

	glBindVertexArray(m->GetArrayObject());
	glDrawElementsInstanced(m->GetType(), static_cast<unsigned int>(m->GetIndicesCount()), GL_UNSIGNED_INT, 0, amount);
	glBindVertexArray(0);
}

void Renderer::DrawInstancedMesh(Mesh* mesh, Shader* s, const std::vector<GLuint>& matTexturesV, const std::vector<GLuint>& matTexturesBumpV, const int& amount)
{
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		OGLRenderer::BindTexture(matTexturesV[i], 0, "diffuseTex", s);
		OGLRenderer::BindTexture(matTexturesBumpV[i], 1, "bumpTex", s);

		mesh->DrawSubMesh(i, amount);
	}
}