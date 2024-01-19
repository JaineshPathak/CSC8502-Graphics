#include "TerrainNode.h"
#include "SceneRenderer.h"

TerrainNode::TerrainNode()
{
	nodeName = "TerrainMain";
	m_TerrainHMap = std::shared_ptr<TerrainHeightmap>(new TerrainHeightmap(TEXTUREDIRCOURSETERRAIN"Terrain_heightmap4.png", 32.0f, 32.0f, 16.0f, 16.0f));
	shader = m_TerrainHMap->GetTerrainShader();
}

void TerrainNode::Draw(const OGLRenderer& r)
{
	shader->SetBool("hasBumpTex", false);
	shader->SetTexture("diffuseSplatmapTex", m_TerrainHMap->GetTerrainTextureSplatmap(), 1);
	shader->SetTexture("diffuseGrassTex", m_TerrainHMap->GetTerrainTextureGrass(), 2);
	shader->SetTexture("diffuseRocksTex", m_TerrainHMap->GetTerrainTextureRocks(), 3);
	shader->SetTexture("diffuseGroundTex", m_TerrainHMap->GetTerrainTextureGround(), 4);
	shader->SetTexture("shadowTex", SceneRenderer::Get()->GetDepthTexture(), 5);

	m_TerrainHMap->Draw();
}

void TerrainNode::DepthDraw(Shader* s)
{
	m_TerrainHMap->Draw();
}
