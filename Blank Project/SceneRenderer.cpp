#include "SceneRenderer.h"
#include "AssetManager.h"
#include "TerrainNode.h"
#include <algorithm>

#include <nclgl/Camera.h>

SceneRenderer::SceneRenderer(Window& parent) : OGLRenderer(parent)
{
	init = Initialize();
	if (!init) return;

	m_CubeMesh = std::shared_ptr<Mesh>(Mesh::GenerateCube());
	m_TerrainNode = std::shared_ptr<TerrainNode>(new TerrainNode());

	InitGLParameters();
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
	if (!InitGLParameters()) return false;

	return true;
}

bool SceneRenderer::InitCamera()
{
	m_Camera = std::shared_ptr<Camera>(new Camera());
	m_Camera->SetDefaultSpeed(850.0f);
	
	return m_Camera != nullptr;
}

bool SceneRenderer::InitGLParameters()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

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
		BindShader(Node->GetShader());
		modelMatrix = Node->GetWorldTransform() * Matrix4::Scale(Node->GetModelScale());

		viewMatrix = m_Camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 60.0f);

		UpdateShaderMatrices();

		glUniform3fv(glGetUniformLocation(Node->GetShader()->GetProgram(), "cameraPos"), 1, (float*)&m_Camera->getPosition());

		/*glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDir"), 1, (float*)&dirLight->GetLightDir());
		glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirColour"), 1, (float*)&dirLight->GetColour());
		glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "lightDirIntensity"), dirLight->GetIntensity());*/

		//glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "enableFog"), enableFog);
		//glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "fogColour"), 1, (float*)&fogColour);

		//SetShaderLight(allPointLights[numPointLights - 1]);		//Sun

		//glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "numPointLights"), numPointLights);
		/*for (size_t i = 0; i < allPointLights.size() && (numPointLights > 0); i++)
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
		}*/

		/*glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowTex"), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);*/

		Node->Draw(*this);

		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
	}
}
