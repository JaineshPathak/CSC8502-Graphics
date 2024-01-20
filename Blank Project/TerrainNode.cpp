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
	shader->SetBool("hasBumpTex", true);
	shader->SetTexture("diffuseSplatmapTex", m_TerrainHMap->GetTerrainTextureSplatmap(), 0);
	shader->SetTexture("diffuseGrassTex", m_TerrainHMap->GetTerrainTextureGrass(), 1);
	shader->SetTexture("bumpGrassTex", m_TerrainHMap->GetTerrainTextureGrassBump(), 2);
	shader->SetTexture("diffuseRocksTex", m_TerrainHMap->GetTerrainTextureRocks(), 3);
	shader->SetTexture("bumpRocksTex", m_TerrainHMap->GetTerrainTextureRocksBump(), 4);
	shader->SetTexture("diffuseGroundTex", m_TerrainHMap->GetTerrainTextureGround(), 5);
	shader->SetTexture("bumpGroundTex", m_TerrainHMap->GetTerrainTextureGroundBump(), 6);
	shader->SetTexture("shadowTex", SceneRenderer::Get()->GetDepthTexture(), 7);

	m_TerrainHMap->Draw();
}

void TerrainNode::DepthDraw(Shader* s)
{
	m_TerrainHMap->Draw();
}
