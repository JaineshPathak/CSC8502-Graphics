#include "SceneRenderer.h"

#include "AssetManager.h"
#include "FileHandler.h"
#include "TerrainNode.h"
#include "TreePropNode.h"
#include "AnimMeshNode.h"
#include "Skybox.h"

#include <nclgl/Camera.h>
#include <nclgl/DirectionalLight.h>

#include <algorithm>

const Vector4 FOG_COLOUR(0.384f, 0.416f, 0.5f, 1.0f);
const Vector3 DIRECTIONAL_LIGHT_DIR(0.5f, -1.0f, 0.0f);
const Vector4 DIRECTIONAL_LIGHT_COLOUR(1.0f, 0.870f, 0.729f, 1.0f);

std::shared_ptr<SceneNode> SceneRenderer::m_RootNode = nullptr;
SceneRenderer* SceneRenderer::m_Instance = nullptr;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

SceneRenderer::SceneRenderer(Window& parent) : OGLRenderer(parent), m_AssetManager(*AssetManager::Get())
{
	m_Instance = this;

	init = Initialize();
	if (!init) return;

	if (m_Camera && m_TerrainNode)
		m_Camera->SetPosition(m_TerrainNode->GetHeightmapSize() * Vector3(0.5f, 4.5f, 0.5f));
}

SceneRenderer::~SceneRenderer(void)
{
	m_Instance = nullptr;
}

void SceneRenderer::RenderScene()
{
	BuildNodeLists(m_RootNode.get());
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	if (m_Skybox) 
		m_Skybox->Draw();

	DrawAllNodes();
	ClearNodeLists();
}

void SceneRenderer::UpdateScene(float DeltaTime)
{
	if (m_Camera) m_Camera->UpdateCamera(DeltaTime);
	if (m_RootNode) m_RootNode->Update(DeltaTime);
}

bool SceneRenderer::Initialize()
{
	if (!InitCamera()) return false;
	if (!InitShaders()) return false;
	if (!InitMeshes()) return false;
	if (!InitMeshMaterials()) return false;
	if (!InitMeshAnimations()) return false;
	if (!InitLights()) return false;
	if (!InitSkybox()) return false;
	if (!InitSceneNodes()) return false;
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
	m_DiffuseAnimShader = m_AssetManager.GetShader("DiffuseAnimShader", SHADERDIRCOURSETERRAIN"CWTexturedSkinVertex.glsl", SHADERDIRCOURSETERRAIN"CWTexturedSkinFragment.glsl");
	m_SkyboxShader = m_AssetManager.GetShader("SkyboxShader", "skyboxVertex.glsl", "skyboxFragment.glsl");

	return  m_TerrainShader->LoadSuccess() && 
			m_DiffuseShader->LoadSuccess() && 
			m_DiffuseAnimShader->LoadSuccess() && 
			m_SkyboxShader->LoadSuccess();
}

bool SceneRenderer::InitMeshes()
{
	m_QuadMesh = std::shared_ptr<Mesh>(Mesh::GenerateQuad());

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

bool SceneRenderer::InitLights()
{
	m_DirLight = std::shared_ptr<DirectionalLight>(new DirectionalLight(DIRECTIONAL_LIGHT_DIR, DIRECTIONAL_LIGHT_COLOUR, Vector4()));
	if (m_DirLight == nullptr) return false;

	if (FileHandler::FileExists(LIGHTSDATAFILE))
	{
		FileHandler::ReadLightDataFile(LIGHTSDATAFILE, *m_DirLight, m_PointLightsList);		
		m_PointLightsNum = (int)m_PointLightsList.size();
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
	m_RootNode = m_TerrainNode;

	SceneNodeProperties rocksProperties("Rock", ROCK2FILE, m_AssetManager.GetMesh("Rocks01"), m_AssetManager.GetMeshMaterial("Rocks01Mat"), false);
	rocksProperties.nodeParent = m_RootNode;

	SceneNodeProperties treesProperties("Tree", TREESFILE, m_AssetManager.GetMesh("Tree01"), m_AssetManager.GetMeshMaterial("Tree01Mat"), true);
	treesProperties.nodeParent = m_RootNode;

	SceneNodeProperties castleProperties("Castle", CASTLEFILE, m_AssetManager.GetMesh("CastleMain"), m_AssetManager.GetMeshMaterial("CastleMainMat"), false);
	castleProperties.nodeParent = m_RootNode;

	SceneNodeProperties castlePillarProperties("CastlePillar", CASTLEPILLARFILE, m_AssetManager.GetMesh("CastlePillar"), m_AssetManager.GetMeshMaterial("CastlePillarMat"), false);
	castlePillarProperties.nodeParent = m_RootNode;

	SceneNodeProperties castleArchProperties("CastleArch", CASTLEARCHFILE, m_AssetManager.GetMesh("CastleArch"), m_AssetManager.GetMeshMaterial("CastleArchMat"), false);
	castleArchProperties.nodeParent = m_RootNode;

	SceneNodeProperties castleBridgeProperties("CastleBridge", CASTLEBRIDGEFILE, m_AssetManager.GetMesh("CastleBridge"), m_AssetManager.GetMeshMaterial("CastleBridgeMat"), false);
	castleBridgeProperties.nodeParent = m_RootNode;

	SceneNodeProperties ruinsProperties("Ruins", RUINSFILE, m_AssetManager.GetMesh("Ruins"), m_AssetManager.GetMeshMaterial("RuinsMat"), false);
	ruinsProperties.nodeParent = m_RootNode;

	SceneNodeProperties crystalAProperties("CrystalA", CRYSTAL01FILE, m_AssetManager.GetMesh("Crystal01"), m_AssetManager.GetMeshMaterial("Crystal01Mat"), false);
	crystalAProperties.nodeParent = m_RootNode;

	SceneNodeProperties crystalBProperties("CrystalB", CRYSTAL02FILE, m_AssetManager.GetMesh("Crystal02"), m_AssetManager.GetMeshMaterial("Crystal02Mat"), false);
	crystalBProperties.nodeParent = m_RootNode;

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
	monsterDudeProp.nodeParent = m_RootNode;

	AnimSceneNodeProperties monsterCrabProp("MonsterCrab", MONSTERCRABFILE, m_AssetManager.GetMesh("MonsterCrab"), m_AssetManager.GetMeshMaterial("MonsterCrabMat"), m_AssetManager.GetMeshAnimation("MonsterCrabAnim"), false);
	monsterCrabProp.nodeParent = m_RootNode;

	SpawnSceneNode(monsterDudeProp);
	SpawnSceneNode(monsterCrabProp);

	return true;
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

void SceneRenderer::DrawAllNodes()
{
	for (const auto& i : m_OpaqueNodesList)
		DrawNode(i);

	for (const auto& i : m_TransparentNodesList)
		DrawNode(i);
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

void SceneRenderer::DrawNode(SceneNode* Node)
{
	if (Node->GetMesh())
	{
		Shader* nodeShader = Node->GetShader();

		nodeShader->Bind();

		modelMatrix = Node->GetWorldTransform() * Matrix4::Scale(Node->GetModelScale());
		viewMatrix = m_Camera->BuildViewMatrix();
		projMatrix = m_Camera->GetProjMatrix();

		nodeShader->SetVector3("cameraPos", m_Camera->getPosition());

		nodeShader->SetMat4("modelMatrix", modelMatrix);
		nodeShader->SetMat4("viewMatrix", viewMatrix);
		nodeShader->SetMat4("projMatrix", projMatrix);

		nodeShader->SetVector3("lightDir", DIRECTIONAL_LIGHT_DIR);
		nodeShader->SetVector4("lightDirColour", DIRECTIONAL_LIGHT_COLOUR);
		nodeShader->SetFloat("lightDirIntensity", 1.0f);

		nodeShader->SetInt("numPointLights", m_PointLightsNum);
		if (m_PointLightsNum > 0)
		{
			for (size_t i = 0; i < m_PointLightsList.size(); i++)
			{
				Light& pointLight = m_PointLightsList[i];

				std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
				std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
				std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
				std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";
				
				nodeShader->SetVector3(lightPosName, pointLight.GetPosition());
				nodeShader->SetVector4(lightColorName, pointLight.GetColour());
				nodeShader->SetFloat(lightRadiusName, pointLight.GetRadius());
				nodeShader->SetFloat(lightIntensityName, pointLight.GetIntensity());
			}
		}

		nodeShader->SetInt("enableFog", true);
		nodeShader->SetVector4("fogColour", FOG_COLOUR);

		Node->Draw(*this);

		nodeShader->UnBind();
	}
}