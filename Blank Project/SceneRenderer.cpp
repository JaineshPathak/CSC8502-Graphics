#include "SceneRenderer.h"

#include "AssetManager.h"
#include "FileHandler.h"
#include "TerrainNode.h"
#include "TreePropNode.h"
#include "AnimMeshNode.h"
#include "LightPointNode.h"
#include "WaterPropNode.h"
#include "Skybox.h"
#include "ShadowBuffer.h"

#include <nclgl/Camera.h>
#include <nclgl/DirectionalLight.h>
#include <nclgl/UniformBuffer.h>

#include <algorithm>

const Vector4 FOG_COLOUR(0.384f, 0.416f, 0.5f, 1.0f);
const Vector3 DIRECTIONAL_LIGHT_DIR(-0.15f, -1.0f, -1.0f);
const Vector4 DIRECTIONAL_LIGHT_COLOUR(1.0f, 0.870f, 0.729f, 1.0f);

const int MAX_POINT_LIGHTS = 100;

const int UBO_MATRIX_BINDING_INDEX = 0;
const int UBO_DIR_LIGHT_BINDING_INDEX = 1;
const int UBO_POINT_LIGHT_BINDING_INDEX = 2;
const int UBO_ENVIRONMENT_BINDING_INDEX = 3;

std::shared_ptr<SceneNode> SceneRenderer::m_RootNode = nullptr;
SceneRenderer* SceneRenderer::m_Instance = nullptr;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

SceneRenderer::SceneRenderer(Window& parent) : OGLRenderer(parent), m_AssetManager(*AssetManager::Get()), m_OrthographicFOV(24.0f)
{
	m_Instance = this;

	//-----------------------------------------------------------
	//Imgui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
	//-----------------------------------------------------------

	init = Initialize();
	if (!init) return;

	//if (m_Camera && m_TerrainNode)
		//m_Camera->SetPosition(m_TerrainNode->GetHeightmapSize() * Vector3(0.5f, 4.5f, 0.5f));

	if (m_TerrainNode)
	{
		m_TerrainNode->SetModelPosition(m_TerrainNode->GetHeightmapSize() * Vector3(-0.5f, -0.5f, -0.5f));
		m_TerrainNode->SetTransform(Matrix4::Translation(m_TerrainNode->GetModelPosition()) * m_TerrainNode->GetRotationMatrix() * Matrix4::Scale(m_TerrainNode->GetModelScale()));
	}
}

SceneRenderer::~SceneRenderer(void)
{
	m_Instance = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void SceneRenderer::RenderScene()
{
	BuildNodeLists(m_RootNode.get());
	SortNodeLists();
	
	DrawShadowDepth();

	glViewport(0, 0, width, height);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	if (m_Skybox) 
		m_Skybox->Draw();

	DrawAllNodes();
	ClearNodeLists();

	//DrawQuadScreen();
	DrawImGui();
}

void SceneRenderer::UpdateScene(float DeltaTime)
{
	if (m_Camera)
	{
		m_Camera->UpdateCamera(DeltaTime);
		m_Camera->BuildViewMatrix();
	}

	if (m_RootNode) m_RootNode->Update(DeltaTime);

	if (m_ShadowBuffer) m_ShadowBuffer->Update(DeltaTime);

	if (Window::GetKeyboard()->KeyHeld(KeyboardKeys::KEYBOARD_UP))
		m_OrthographicFOV += DeltaTime;
	
	if (Window::GetKeyboard()->KeyHeld(KeyboardKeys::KEYBOARD_DOWN))
		m_OrthographicFOV -= DeltaTime;

	UpdateUBOData();

	//std::cout << "Ortho FOV: " << m_OrthographicFOV << std::endl;
}

bool SceneRenderer::Initialize()
{
	if (!InitCamera()) return false;
	if (!InitShaders()) return false;
	if (!InitMeshes()) return false;
	if (!InitMeshMaterials()) return false;
	if (!InitMeshAnimations()) return false;
	if (!InitBuffers()) return false;
	if (!InitSkybox()) return false;
	if (!InitSceneNodes()) return false;
	if (!InitLights()) return false;
	if (!InitGLParameters()) return false;

	return true;
}

bool SceneRenderer::InitCamera()
{
	m_Camera = std::shared_ptr<Camera>(new Camera());
	m_Camera->SetDefaultSpeed(850.0f);
	m_Camera->SetProjMatrix(1.0f, 15000.0f, (float)width, (float)height);
	
	return m_Camera != nullptr;
}

bool SceneRenderer::InitShaders()
{
	m_TerrainShader = m_AssetManager.GetShader("TerrainShader", SHADERDIRCOURSETERRAIN"CWTexturedVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWTerrainFragv2.glsl");
	m_DiffuseShader = m_AssetManager.GetShader("DiffuseShader", SHADERDIRCOURSETERRAIN"CWTexturedVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragmentv2.glsl");
	m_DiffuseAnimShader = m_AssetManager.GetShader("DiffuseAnimShader", SHADERDIRCOURSETERRAIN"CWTexturedSkinVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWTexturedFragmentv2.glsl");
	m_SkyboxShader = m_AssetManager.GetShader("SkyboxShader", SHADERDIRCOURSETERRAIN"CWSkyboxVertex.glsl", "skyboxFragment.glsl");
	m_DepthShadowShader = m_AssetManager.GetShader("DepthShader", "DepthShadowBufferVert.glsl", "DepthShadowBufferFrag.glsl");
	m_QuadShader = m_AssetManager.GetShader("QuadDepthShader", "DepthQuadBufferVert.glsl", "DepthQuadBufferFrag.glsl");
	m_ReflectShader = m_AssetManager.GetShader("WaterShader", SHADERDIRCOURSETERRAIN"CWReflectVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWReflectFragmentv2.glsl");

	return  m_TerrainShader->LoadSuccess() && 
			m_DiffuseShader->LoadSuccess() && 
			m_DiffuseAnimShader->LoadSuccess() && 
			m_SkyboxShader->LoadSuccess() &&
			m_DepthShadowShader->LoadSuccess() &&
			m_QuadShader->LoadSuccess() &&
			m_ReflectShader->LoadSuccess();
}

bool SceneRenderer::InitMeshes()
{
	m_QuadMesh = std::shared_ptr<Mesh>(Mesh::GenerateQuad());
	m_QuadMiniMesh = std::shared_ptr<Mesh>(Mesh::GenerateQuadMini());
	m_CubeMesh = std::shared_ptr<Mesh>(Mesh::GenerateCube());
	m_PointMesh = std::shared_ptr<Mesh>(Mesh::GeneratePoint());

	m_AssetManager.GetMesh("Rocks01", MESHDIRCOURSE"Rocks/Mesh_Rock5D.msh");
	m_AssetManager.GetMesh("Tree01", MESHDIRCOURSE"Trees/Tree_01.msh");
	m_AssetManager.GetMesh("CastleMain", MESHDIRCOURSE"Castle/Mesh_CastleMain.msh");
	m_AssetManager.GetMesh("CastlePillar", MESHDIRCOURSE"Castle/Mesh_CastlePillar.msh");
	m_AssetManager.GetMesh("CastleArch", MESHDIRCOURSE"Castle/Mesh_CastleArch.msh");
	m_AssetManager.GetMesh("CastleBridge", MESHDIRCOURSE"Castle/Mesh_Bridge.msh");
	m_AssetManager.GetMesh("Ruins", MESHDIRCOURSE"Ruins/Mesh_RuinsMain.msh");
	m_AssetManager.GetMesh("Crystal01", MESHDIRCOURSE"Crystals/Mesh_Crystal_01.msh");
	m_AssetManager.GetMesh("Crystal02", MESHDIRCOURSE"Crystals/Mesh_Crystal_02.msh");
	m_AssetManager.GetMesh("MonsterDude", MESHDIRCOURSE"Monsters/Monster_Dude.msh");
	m_AssetManager.GetMesh("MonsterCrab", MESHDIRCOURSE"Monsters/Monster_Crab.msh");

	return true;
}

bool SceneRenderer::InitMeshMaterials()
{
	m_AssetManager.GetMeshMaterial("Rocks01Mat", MESHDIRCOURSE"Rocks/Mesh_Rock5D.mat");
	m_AssetManager.GetMeshMaterial("Tree01Mat", MESHDIRCOURSE"Trees/Tree_01.mat");
	m_AssetManager.GetMeshMaterial("CastleMainMat", MESHDIRCOURSE"Castle/Mesh_CastleMain.mat");
	m_AssetManager.GetMeshMaterial("CastlePillarMat", MESHDIRCOURSE"Castle/Mesh_CastlePillar.mat");
	m_AssetManager.GetMeshMaterial("CastleArchMat", MESHDIRCOURSE"Castle/Mesh_CastleArch.mat");
	m_AssetManager.GetMeshMaterial("CastleBridgeMat", MESHDIRCOURSE"Castle/Mesh_Bridge.mat");
	m_AssetManager.GetMeshMaterial("RuinsMat", MESHDIRCOURSE"Ruins/Mesh_RuinsMain.mat");
	m_AssetManager.GetMeshMaterial("Crystal01Mat", MESHDIRCOURSE"Crystals/Mesh_Crystal_01.mat");
	m_AssetManager.GetMeshMaterial("Crystal02Mat", MESHDIRCOURSE"Crystals/Mesh_Crystal_02.mat");
	m_AssetManager.GetMeshMaterial("MonsterDudeMat", MESHDIRCOURSE"Monsters/Monster_Dude.mat");
	m_AssetManager.GetMeshMaterial("MonsterCrabMat", MESHDIRCOURSE"Monsters/Monster_Crab.mat");

	return true;
}

bool SceneRenderer::InitMeshAnimations()
{
	m_AssetManager.GetMeshAnimation("MonsterDudeAnim", MESHDIRCOURSE"Monsters/Monster_Dude.anm");
	m_AssetManager.GetMeshAnimation("MonsterCrabAnim", MESHDIRCOURSE"Monsters/Monster_Crab.anm");

	return true;
}

bool SceneRenderer::InitBuffers()
{
	m_MatrixUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(sizeof(Matrix4) * 2, NULL, GL_STATIC_DRAW, UBO_MATRIX_BINDING_INDEX, 0));
	m_DirLightUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(sizeof(DirectionalLightStruct), NULL, GL_DYNAMIC_DRAW, UBO_DIR_LIGHT_BINDING_INDEX, 0));
	m_PointLightUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer((MAX_POINT_LIGHTS * sizeof(PointLightStruct)) + (sizeof(int) * 4), NULL, GL_DYNAMIC_DRAW, UBO_POINT_LIGHT_BINDING_INDEX, 0));
	m_EnvironmentUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(sizeof(EnvironmentDataStruct), NULL, GL_STATIC_DRAW, UBO_ENVIRONMENT_BINDING_INDEX, 0));

	m_ShadowBuffer = std::shared_ptr<ShadowBuffer>(new ShadowBuffer(2048, 2048));

	return  m_MatrixUBO->IsInitialized() &&
			m_DirLightUBO->IsInitialized() &&
			m_PointLightUBO->IsInitialized() &&
			m_EnvironmentUBO->IsInitialized() &&
			m_ShadowBuffer != nullptr;
}

bool SceneRenderer::InitLights()
{
	m_DirLight = std::shared_ptr<DirectionalLight>(new DirectionalLight(DIRECTIONAL_LIGHT_DIR, DIRECTIONAL_LIGHT_COLOUR, Vector4()));
	if (m_DirLight == nullptr) return false;

	m_DirLightStruct = DirectionalLightStruct();
	m_DirLightStruct.lightDirection = Vector4(m_DirLight->GetLightDir());
	m_DirLightStruct.lightDirection.w = m_DirLight->GetIntensity();
	m_DirLightStruct.lightColor = m_DirLight->GetColour();

	if (FileHandler::FileExists(LIGHTSDATAFILE))
	{
		//FileHandler::ReadLightDataFile(LIGHTSDATAFILE, *m_DirLight, m_PointLightsList);
		std::vector<float> pLightIntensityV;
		std::vector<float> pLightRadiusV;
		std::vector<Vector3> pLightPosV;
		std::vector<Vector4> pLightColorV;
		std::vector<Vector4> pLightSpecColorV;

		FileHandler::ReadLightDataFile(LIGHTSDATAFILE, pLightIntensityV, pLightRadiusV, pLightPosV, pLightColorV, pLightSpecColorV);
		if (pLightIntensityV.size() > 0)
		{
			for (int i = 0; i < pLightIntensityV.size(); i++)
			{
				std::shared_ptr<LightPointNode> pointLightNode = std::shared_ptr<LightPointNode>(new LightPointNode());
				pointLightNode->SetLightRadius(pLightRadiusV[i]);
				pointLightNode->SetLightIntensity(pLightIntensityV[i]);
				pointLightNode->SetPosition(pLightPosV[i]);
				pointLightNode->SetLightColour(pLightColorV[i]);
				pointLightNode->SetLightSpecularColour(pLightSpecColorV[i]);

				m_PointLightsList.emplace_back(pointLightNode);

				if (m_TerrainNode) 
					m_TerrainNode->AddChild(pointLightNode.get());

				//Point Light UBO
				PointLightStruct pLight = PointLightStruct();
				pLight.lightPosition = Vector4(pLightPosV[i]);
				pLight.lightColor = pLightColorV[i];
				pLight.lightRadiusIntensityData = Vector4(pLightRadiusV[i], pLightIntensityV[i], 0.0f, 1.0f);

				m_PointLightStructList.push_back(pLight);
			}

			m_PointLightsNum = (int)m_PointLightsList.size();
		}
	}

	return true;
}

bool SceneRenderer::InitSkybox()
{
	m_Skybox = std::shared_ptr<Skybox>(new Skybox());
	if (m_Skybox != nullptr)
	{
		std::cout << "Skybox: Ready" << std::endl;
		return true;
	}

	return false;
}

bool SceneRenderer::InitGLParameters()
{
	//Fog Data
	m_EnvironmentDataStruct = EnvironmentDataStruct(true, 0.00015f, 1.5f, FOG_COLOUR);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return true;
}

bool SceneRenderer::InitSceneNodes()
{
	//Spawn the Root Nodes
	m_TerrainNode = std::shared_ptr<TerrainNode>(new TerrainNode());

	m_RootNode = std::shared_ptr<SceneNode>(new SceneNode("Root"));
	m_RootNode->AddChild(m_TerrainNode.get());

	SceneNodeProperties rocksProperties("Rock", ROCK2FILE, m_AssetManager.GetMesh("Rocks01"), m_AssetManager.GetMeshMaterial("Rocks01Mat"), false);
	rocksProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties treesProperties("Tree", TREESFILE, m_AssetManager.GetMesh("Tree01"), m_AssetManager.GetMeshMaterial("Tree01Mat"), true);
	treesProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties castleProperties("Castle", CASTLEFILE, m_AssetManager.GetMesh("CastleMain"), m_AssetManager.GetMeshMaterial("CastleMainMat"), false);
	castleProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties castlePillarProperties("CastlePillar", CASTLEPILLARFILE, m_AssetManager.GetMesh("CastlePillar"), m_AssetManager.GetMeshMaterial("CastlePillarMat"), false);
	castlePillarProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties castleArchProperties("CastleArch", CASTLEARCHFILE, m_AssetManager.GetMesh("CastleArch"), m_AssetManager.GetMeshMaterial("CastleArchMat"), false);
	castleArchProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties castleBridgeProperties("CastleBridge", CASTLEBRIDGEFILE, m_AssetManager.GetMesh("CastleBridge"), m_AssetManager.GetMeshMaterial("CastleBridgeMat"), false);
	castleBridgeProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties ruinsProperties("Ruins", RUINSFILE, m_AssetManager.GetMesh("Ruins"), m_AssetManager.GetMeshMaterial("RuinsMat"), false);
	ruinsProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties crystalAProperties("CrystalA", CRYSTAL01FILE, m_AssetManager.GetMesh("Crystal01"), m_AssetManager.GetMeshMaterial("Crystal01Mat"), false);
	crystalAProperties.nodeParent = m_TerrainNode;

	SceneNodeProperties crystalBProperties("CrystalB", CRYSTAL02FILE, m_AssetManager.GetMesh("Crystal02"), m_AssetManager.GetMeshMaterial("Crystal02Mat"), false);
	crystalBProperties.nodeParent = m_TerrainNode;

	SpawnSceneNode(rocksProperties);
	SpawnSceneNode(treesProperties);
	SpawnSceneNode(castleProperties);
	SpawnSceneNode(castlePillarProperties);
	SpawnSceneNode(castleArchProperties);
	SpawnSceneNode(castleBridgeProperties);
	SpawnSceneNode(ruinsProperties);
	SpawnSceneNode(crystalAProperties);
	SpawnSceneNode(crystalBProperties);

	AnimSceneNodeProperties monsterDudeProp("MonsterDude", MONSTERDUDEFILE, m_AssetManager.GetMesh("MonsterDude"), m_AssetManager.GetMeshMaterial("MonsterDudeMat"), m_AssetManager.GetMeshAnimation("MonsterDudeAnim"), false);
	monsterDudeProp.nodeParent = m_TerrainNode;

	AnimSceneNodeProperties monsterCrabProp("MonsterCrab", MONSTERCRABFILE, m_AssetManager.GetMesh("MonsterCrab"), m_AssetManager.GetMeshMaterial("MonsterCrabMat"), m_AssetManager.GetMeshAnimation("MonsterCrabAnim"), false);
	monsterCrabProp.nodeParent = m_TerrainNode;

	SpawnSceneNode(monsterDudeProp);
	SpawnSceneNode(monsterCrabProp);

	//Water Prop
	WaterPropNode* waterPropNode = new WaterPropNode();
	if (waterPropNode && m_TerrainNode)
	{
		waterPropNode->SetSkyboxTexID(m_Skybox->GetSkyboxCube());
		waterPropNode->SetModelPosition(m_TerrainNode->GetHeightmapSize() * Vector3(0.5f, 0.2355f, 0.5f));
		waterPropNode->SetModelRotation(Vector3(90.0f, 0, 0));
		waterPropNode->SetModelScale(Vector3(60.0f, 60.0f, 60.0f));
		waterPropNode->SetTransform(Matrix4::Translation(waterPropNode->GetModelPosition()) * Matrix4::Scale(waterPropNode->GetModelScale()) * waterPropNode->GetRotationMatrix());
		
		m_TerrainNode->AddChild(waterPropNode);
	}

	return true;
}

void SceneRenderer::UpdateUBOData()
{
	//Camera's View and Projection Matrix
	if (m_MatrixUBO == nullptr || !m_MatrixUBO->IsInitialized()) return;

	m_MatrixUBO->Bind();
	m_MatrixUBO->BindSubData(0, sizeof(Matrix4), m_Camera->GetProjMatrix().values);
	m_MatrixUBO->BindSubData(sizeof(Matrix4), sizeof(Matrix4), m_Camera->GetViewMatrix().values);
	m_MatrixUBO->Unbind();

	//Directional Light's
	if (m_DirLightUBO == nullptr || !m_DirLightUBO->IsInitialized()) return;

	m_DirLightStruct.lightDirection = Vector4(m_DirLight->GetLightDir());
	m_DirLightStruct.lightDirection.w = m_DirLight->GetIntensity();
	m_DirLightStruct.lightColor = m_DirLight->GetColour();

	m_DirLightUBO->Bind();
	m_DirLightUBO->BindSubData(0, sizeof(DirectionalLightStruct), &m_DirLightStruct);
	m_DirLightUBO->Unbind();

	//Point Light's
	if (m_PointLightUBO == nullptr || !m_PointLightUBO->IsInitialized()) return;

	if (m_PointLightsNum > 0)
	{
		for (size_t i = 0; i < m_PointLightsNum; i++)
		{
			m_PointLightStructList[i].lightPosition = Vector4(m_PointLightsList[i]->GetLightPosition());
			m_PointLightStructList[i].lightColor = m_PointLightsList[i]->GetLightColour();
			m_PointLightStructList[i].lightRadiusIntensityData = Vector4(m_PointLightsList[i]->GetLightRadius(), m_PointLightsList[i]->GetLightIntensity(), 0.0f, 1.0f);
		}
	}

	m_PointLightUBO->Bind();
	m_PointLightUBO->BindSubData(0, sizeof(int), &m_PointLightsNum);
	m_PointLightUBO->BindSubData(sizeof(int) * 4, sizeof(PointLightStruct) * (int)m_PointLightStructList.size(), m_PointLightStructList.data());
	m_PointLightUBO->Unbind();

	//Environment's Fog
	if (m_EnvironmentUBO == nullptr || !m_EnvironmentUBO->IsInitialized()) return;

	m_EnvironmentUBO->Bind();
	m_EnvironmentUBO->BindSubData(0, sizeof(EnvironmentDataStruct), &m_EnvironmentDataStruct);
	m_EnvironmentUBO->Unbind();
}

void SceneRenderer::SpawnSceneNode(const SceneNodeProperties& nodeProp)
{
	if (!FileHandler::FileExists(nodeProp.nodeFilePath)) return;

	std::vector<Vector3> posV, rotV, scaleV;
	FileHandler::ReadPropDataFromFile(nodeProp.nodeFilePath, posV, rotV, scaleV);

	for (int i = 0; i < posV.size(); i++)
	{
		TreePropNode* newNode = new TreePropNode(nodeProp.nodeMesh.get(), nodeProp.nodeMeshMaterial.get(), m_DiffuseShader.get(), TEXTUREDIR);
		newNode->nodeName = nodeProp.nodeName + std::to_string(i);
		newNode->SetModelPosition(posV[i]);
		newNode->SetModelRotation(rotV[i]);
		newNode->SetModelScale(scaleV[i]);
		newNode->SetTransform(Matrix4::Translation(newNode->GetModelPosition()) * newNode->GetRotationMatrix() * Matrix4::Scale(newNode->GetModelScale()));
		
		if (nodeProp.isTransparent)
			newNode->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));

		if(nodeProp.nodeParent != nullptr)
			nodeProp.nodeParent->AddChild(newNode);
	}
}

void SceneRenderer::SpawnSceneNode(const AnimSceneNodeProperties& nodeProp)
{
	if (!FileHandler::FileExists(nodeProp.nodeFilePath)) return;

	std::vector<Vector3> posV, rotV, scaleV;
	FileHandler::ReadPropDataFromFile(nodeProp.nodeFilePath, posV, rotV, scaleV);

	for (int i = 0; i < posV.size(); i++)
	{
		AnimMeshNode* newNode = new AnimMeshNode(m_DiffuseAnimShader.get(), nodeProp.nodeMesh.get(), nodeProp.nodeMeshAnimation.get(), nodeProp.nodeMeshMaterial.get(), TEXTUREDIR);
		newNode->nodeName = nodeProp.nodeName + std::to_string(i);
		newNode->SetModelPosition(posV[i]);
		newNode->SetModelRotation(rotV[i]);
		newNode->SetModelScale(scaleV[i]);
		newNode->SetTransform(Matrix4::Translation(newNode->GetModelPosition()) * newNode->GetRotationMatrix() * Matrix4::Scale(newNode->GetModelScale()));

		if (nodeProp.isTransparent)
			newNode->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));

		if (nodeProp.nodeParent != nullptr)
			nodeProp.nodeParent->AddChild(newNode);
	}
}

void SceneRenderer::DrawShadowDepth()
{
	if (m_ShadowBuffer == nullptr) return;
	if (m_DepthShadowShader == nullptr) return;

	m_ShadowBuffer->Bind();
	m_DepthShadowShader->Bind();

	float near_plane = -1000.0f, far_plane = 10000.0f;
	float left_plane = -100.0f * m_OrthographicFOV;
	float right_plane = 100.0f * m_OrthographicFOV;
	float top_plane = 100.0f * m_OrthographicFOV;
	float bottom_plane = -100.0f * m_OrthographicFOV;
	m_LightSpaceProj = Matrix4::Orthographic(near_plane, far_plane, right_plane, left_plane, top_plane, bottom_plane);
	m_LightSpaceView = Matrix4::BuildViewMatrix(-m_DirLight->GetLightDir(), Vector3(0, 0, 0), Vector3(0.0f, 1.0f, 0.0f));
	//Matrix4 lightView = Matrix4::BuildViewMatrix(m_Camera->getPosition(), m_Camera->getPosition() + m_Camera->GetForward(), Vector3(0.0f, 1.0f, 0.0f));
	//Matrix4 lightView = Matrix4::BuildViewMatrix(m_TerrainNode->GetHeightmapSize() * Vector3(0.5f, 0.5f, 0.5f), m_DirLight->GetLightDir(), Vector3(0.0f, 1.0f, 0.0f));

	//Creating Ortho Projection based on Shadow box dimensions
	/*m_LightSpaceProj.ToIdentity();
	m_LightSpaceProj.values[0] = 2.0f / m_ShadowBuffer->GetBoxWidth();
	m_LightSpaceProj.values[5] = 2.0f / m_ShadowBuffer->GetBoxHeight();
	m_LightSpaceProj.values[10] = -2.0f / m_ShadowBuffer->GetBoxLength();
	m_LightSpaceProj.values[15] = 1.0f;

	//Update the light's view matrix
	const Vector3 boxCenter = m_ShadowBuffer->GetBoxCenter();
	const Vector3 lightDir = m_DirLight->GetLightDir();
	lightDir.Normalised();
	
	float pitch = std::asinf(-lightDir.y);
	pitch = RadToDeg(pitch);
	float yaw = std::atan2f(lightDir.x, lightDir.z);
	yaw = RadToDeg(yaw);
	//yaw = lightDir.z > 0 ? yaw - 180.0f : yaw;
	//float pitch = (float)std::acos(lightDir2D);
	//float yaw = RadToDeg(std::atan(lightDir.x / lightDir.z));
	//yaw = lightDir.z > 0 ? yaw - 180.0f : yaw;
	
	//m_LightSpaceView = Matrix4::Rotation(-pitch, Vector3(1.0f, 0.0f, 0.0f)) * Matrix4::Rotation(-yaw, Vector3(0.0f, 1.0f, 0.0f)) * Matrix4::Translation(-boxCenter);
	//m_LightSpaceView = Matrix4::BuildViewMatrix(-lightDir, boxCenter + lightDir, Vector3(0, 1, 0));*/

	m_LightSpaceMatrix = m_LightSpaceProj * m_LightSpaceView;

	m_DepthShadowShader->SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);

	glClear(GL_DEPTH_BUFFER_BIT);
	DrawAllNodes(true);

	m_DepthShadowShader->UnBind();
	m_ShadowBuffer->UnBind();
}

void SceneRenderer::DrawAllNodes(const bool& isDepth)
{
	for (const auto& i : m_OpaqueNodesList)
		isDepth ? DrawDepthNode(i) : DrawNode(i);
	
	for (const auto& i : m_TransparentNodesList)
		isDepth ? DrawDepthNode(i) : DrawNode(i);
}

void SceneRenderer::BuildNodeLists(SceneNode* fromNode)
{
	if (fromNode == nullptr) return;
	if (m_Camera == nullptr) return;

	Vector3 dir = fromNode->GetWorldTransform().GetPositionVector() - m_Camera->getPosition();
	fromNode->SetCameraDistance(Vector3::Dot(dir, dir));

	if (fromNode->GetColour().w < 1.0f)
		m_TransparentNodesList.push_back(fromNode);
	else
		m_OpaqueNodesList.push_back(fromNode);

	for (vector<SceneNode*>::const_iterator i = fromNode->GetChildIteratorStart(); i != fromNode->GetChildIteratorEnd(); ++i)
		BuildNodeLists((*i));
}

void SceneRenderer::SortNodeLists()
{
	std::sort(m_TransparentNodesList.rbegin(), m_TransparentNodesList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(m_OpaqueNodesList.begin(), m_OpaqueNodesList.end(), SceneNode::CompareByCameraDistance);
}

void SceneRenderer::ClearNodeLists()
{
	m_TransparentNodesList.clear();
	
	m_OpaqueNodesList.clear();
	m_OpaqueNodesList.push_back(m_TerrainNode.get());
}

void SceneRenderer::DrawImGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::CollapsingHeader("Directional Light"))
	{
		Vector3 lightDir = m_DirLight->GetLightDir();
		if (ImGui::DragFloat3("Dir", (float*)&lightDir, 0.01f, -1.0f, 1.0f))
			m_DirLight->SetLightDir(lightDir);
	}

	if (ImGui::CollapsingHeader("Terrain"))
	{
		Vector3 terrainPos = m_TerrainNode->GetModelPosition();
		if (ImGui::DragFloat3("Pos", (float*)&terrainPos))
		{
			m_TerrainNode->SetModelPosition(terrainPos);
			m_TerrainNode->SetTransform(Matrix4::Translation(m_TerrainNode->GetModelPosition()) * m_TerrainNode->GetRotationMatrix() * Matrix4::Scale(m_TerrainNode->GetModelScale()));
		}
	}

	if (ImGui::CollapsingHeader("Environment Data"))
	{
		bool fogEnabled = (bool)m_EnvironmentDataStruct.fogData.x;
		if(ImGui::Checkbox("Fog", &fogEnabled))
			m_EnvironmentDataStruct.fogData.x = (float)fogEnabled;

		ImGui::DragFloat("Fog Density", &m_EnvironmentDataStruct.fogData.y, 0.00001f, 0.0f, 10.0f, "%.5f");
		ImGui::DragFloat("Fog Gradient", &m_EnvironmentDataStruct.fogData.z, 0.01f, -10.0f, 10.0f);
		ImGui::ColorEdit4("Fog Colour", (float*)& m_EnvironmentDataStruct.fogColor);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneRenderer::DrawNode(SceneNode* Node)
{
	if (Node->GetMesh())
	{
		Shader* nodeShader = Node->GetShader();

		nodeShader->Bind();

		modelMatrix = Node->GetWorldTransform() * Matrix4::Scale(Node->GetModelScale());
		//viewMatrix = m_Camera->GetViewMatrix();
		//projMatrix = m_Camera->GetProjMatrix();
		//viewMatrix = Matrix4::BuildViewMatrix(m_Camera->getPosition(), m_TerrainNode->GetHeightmapSize() * Vector3(0.5f, 2.0f, 0.5f), Vector3(0.0f, 1.0f, 0.0f));
		//projMatrix = Matrix4::Orthographic(-1000.0f, 10000.0f, 100.0f * m_OrthographicFOV, -100.0f * m_OrthographicFOV, 100.0f * m_OrthographicFOV, -100.0f * m_OrthographicFOV);

		nodeShader->SetVector3("cameraPos", m_Camera->getPosition());

		nodeShader->SetMat4("modelMatrix", modelMatrix);
		nodeShader->SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);

		Node->Draw(*this);

		nodeShader->UnBind();
	}
}

void SceneRenderer::DrawDepthNode(SceneNode* Node, bool isTransparentNodes)
{
	if (Node != nullptr && Node->GetMesh())
	{
		if (isTransparentNodes)
			glDisable(GL_CULL_FACE);

		modelMatrix = Node->GetWorldTransform() * Matrix4::Scale(Node->GetModelScale());

		m_DepthShadowShader->SetMat4("modelMatrix", modelMatrix);

		AnimMeshNode* animNode = dynamic_cast<AnimMeshNode*>(Node);
		m_DepthShadowShader->SetBool("isAnimated", animNode != nullptr);
		if (animNode != nullptr)
		{
			animNode->CalcFrameMatrices();
			
			int j = glGetUniformLocation(m_DepthShadowShader->GetProgram(), "joints");
			glUniformMatrix4fv(j, (GLsizei)animNode->GetFrameMatrices().size(), false, (float*)animNode->GetFrameMatrices().data());			
		}
		
		Node->DepthDraw(m_DepthShadowShader.get());

		if (isTransparentNodes)
			glEnable(GL_CULL_FACE);

		//Node->GetMesh()->Draw();

		/*viewMatrix = m_Camera->GetViewMatrix();
		projMatrix = m_Camera->GetProjMatrix();

		m_DepthShadowShader->SetMat4("modelMatrix", modelMatrix);
		m_DepthShadowShader->SetMat4("viewMatrix", viewMatrix);
		m_DepthShadowShader->SetMat4("projMatrix", projMatrix);*/		
	}
}

void SceneRenderer::DrawQuadScreen()
{
	m_QuadShader->Bind();

	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();

	m_QuadShader->SetTexture("diffuseTex", m_ShadowBuffer->GetDepthTexture(), 0);

	m_QuadMiniMesh->Draw();

	m_QuadShader->UnBind();
}

unsigned int SceneRenderer::GetDepthTexture() const
{
	return m_ShadowBuffer->GetDepthTexture();
}
