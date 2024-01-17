#include "SceneRenderer.h"

#include "AssetManager.h"
#include "FileHandler.h"
#include "TerrainNode.h"

#include <nclgl/Camera.h>
#include <nclgl/DirectionalLight.h>

#include <algorithm>

const Vector4 FOG_COLOUR(0.384f, 0.416f, 0.5f, 1.0f);
const Vector3 DIRECTIONAL_LIGHT_DIR(1.0f, 0.0f, 0.0f);
const Vector4 DIRECTIONAL_LIGHT_COLOUR(1.0f, 0.870f, 0.729f, 1.0f);

SceneRenderer::SceneRenderer(Window& parent) : OGLRenderer(parent)
{
	init = Initialize();
	if (!init) return;

	m_TerrainNode = std::shared_ptr<TerrainNode>(new TerrainNode());
}

SceneRenderer::~SceneRenderer(void)
{
}

void SceneRenderer::RenderScene()
{
	BuildNodeLists(m_TerrainNode.get());
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawAllNodes();
	ClearNodeLists();
}

void SceneRenderer::UpdateScene(float DeltaTime)
{
	if (m_Camera) m_Camera->UpdateCamera(DeltaTime);
}

bool SceneRenderer::Initialize()
{
	if (!InitCamera()) return false;
	if (!InitMeshes()) return false;
	if (!InitLights()) return false;
	if (!InitGLParameters()) return false;

	return true;
}

bool SceneRenderer::InitCamera()
{
	m_Camera = std::shared_ptr<Camera>(new Camera());
	m_Camera->SetDefaultSpeed(850.0f);
	
	return m_Camera != nullptr;
}

bool SceneRenderer::InitMeshes()
{
	AssetManager::Get()->GetMesh("Rocks01", MESHDIRCOURSE"Rocks/Mesh_Rock5D.msh");
	AssetManager::Get()->GetMesh("Tree01", MESHDIRCOURSE"Trees/Tree_01.msh");
	AssetManager::Get()->GetMesh("CastleMain", MESHDIRCOURSE"Castle/Mesh_CastleMain.msh");
	AssetManager::Get()->GetMesh("CastlePillar", MESHDIRCOURSE"Castle/Mesh_CastlePillar.msh");
	AssetManager::Get()->GetMesh("CastleArch", MESHDIRCOURSE"Castle/Mesh_CastleArch.msh");
	AssetManager::Get()->GetMesh("CastleBridge", MESHDIRCOURSE"Castle/Mesh_Bridge.msh");
	AssetManager::Get()->GetMesh("Ruins", MESHDIRCOURSE"Ruins/Mesh_RuinsMain.msh");
	AssetManager::Get()->GetMesh("Crystal01", MESHDIRCOURSE"Crystals/Mesh_Crystal_01.msh");
	AssetManager::Get()->GetMesh("Crystal02", MESHDIRCOURSE"Crystals/Mesh_Crystal_02.msh");
	AssetManager::Get()->GetMesh("MonsterDude", MESHDIRCOURSE"Monsters/Monster_Dude.msh");
	AssetManager::Get()->GetMesh("MonsterCrab", MESHDIRCOURSE"Monsters/Monster_Crab.msh");

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
		Shader& nodeShader = *Node->GetShader();

		nodeShader.Bind();

		modelMatrix = Node->GetWorldTransform() * Matrix4::Scale(Node->GetModelScale());
		viewMatrix = m_Camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

		nodeShader.SetVector3("cameraPos", m_Camera->getPosition());
		nodeShader.SetMat4("modelMatrix", modelMatrix);
		nodeShader.SetMat4("viewMatrix", viewMatrix);
		nodeShader.SetMat4("projMatrix", projMatrix);

		nodeShader.SetVector3("lightDir", DIRECTIONAL_LIGHT_DIR);
		nodeShader.SetVector4("lightDirColour", DIRECTIONAL_LIGHT_COLOUR);
		nodeShader.SetFloat("lightDirIntensity", 1.0f);

		nodeShader.SetInt("numPointLights", m_PointLightsNum);
		if (m_PointLightsNum > 0)
		{
			for (size_t i = 0; i < m_PointLightsList.size(); i++)
			{
				Light& pointLight = m_PointLightsList[i];

				std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
				std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
				std::string lightSpecularColourName = "pointLightSpecularColour[" + std::to_string(i) + "]";
				std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
				std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";
				
				nodeShader.SetVector3(lightPosName, pointLight.GetPosition());
				nodeShader.SetVector4(lightColorName, pointLight.GetColour());
				nodeShader.SetVector4(lightSpecularColourName, pointLight.GetSpecularColour());
				nodeShader.SetFloat(lightRadiusName, pointLight.GetRadius());
				nodeShader.SetFloat(lightIntensityName, pointLight.GetIntensity());
			}
		}

		nodeShader.SetInt("enableFog", false);
		nodeShader.SetVector4("fogColour", FOG_COLOUR);

		/*glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowTex"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);*/

		Node->Draw(*this);

		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
		nodeShader.UnBind();
	}
}
