#include "WaterPropNode.h"
#include "SceneRenderer.h"
#include "AssetManager.h"

WaterPropNode::WaterPropNode() : m_SkyboxTexID(-1)
{
	nodeName = "WaterNode";

	m_AssetManager = AssetManager::Get();

	mesh = SceneRenderer::Get()->GetQuadMesh().get();
	shader = m_AssetManager->GetShader("WaterShader").get();

	m_WaterTexID = m_AssetManager->GetTexture("WaterDiffuseTex", TEXTUREDIR"water.tga");
	m_WaterBumpTexID = m_AssetManager->GetTexture("WaterBumpTex", TEXTUREDIR"waterbump.png");

	OGLRenderer::SetTextureRepeating(m_WaterTexID, true);
	OGLRenderer::SetTextureRepeating(m_WaterBumpTexID, true);

	m_WaterTexMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f));
}

WaterPropNode::~WaterPropNode()
{
	glDeleteTextures(1, &m_WaterTexID);
	glDeleteTextures(1, &m_WaterBumpTexID);
}

//void WaterPropNode::Update(float DeltaTime)
//{
//	SceneNode::Update(DeltaTime);
//}

void WaterPropNode::Draw(const OGLRenderer& r)
{
	glDisable(GL_CULL_FACE);

	shader->SetMat4("textureMatrix", m_WaterTexMatrix);
	shader->SetBool("hasBumpTex", true);
	shader->SetTexture("diffuseTex", m_WaterTexID, 0);
	shader->SetTexture("bumpTex", m_WaterBumpTexID, 1);
	shader->SetTextureCubeMap("cubeTex", m_SkyboxTexID, 2);
	shader->SetTexture("shadowTex", SceneRenderer::Get()->GetDepthTexture(), 3);

	mesh->Draw();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void WaterPropNode::DepthDraw(Shader* s)
{
	s->SetTexture("diffuseTex", m_WaterTexID, 0);

	if (mesh)
		mesh->Draw();
}
