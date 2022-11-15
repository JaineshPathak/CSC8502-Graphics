#include "TerrainNode.h"

TerrainNode::TerrainNode()
{
	nodeName = "TerrainMain";
	terrainHMap = new TerrainHeightmap(TEXTUREDIRCOURSETERRAIN"Terrain_heightmap4.png", 32.0f, 32.0f, 16.0f, 16.0f);
	shader = terrainHMap->GetTerrainShader();
}

TerrainNode::~TerrainNode()
{
	delete terrainHMap;
}

void TerrainNode::Draw(const OGLRenderer& r)
{
	OGLRenderer::BindTexture(terrainHMap->GetTerrainTextureSplatmap(), 0, "diffuseSplatmapTex", shader);
	OGLRenderer::BindTexture(terrainHMap->GetTerrainTextureGrass(), 1, "diffuseGrassTex", shader);
	OGLRenderer::BindTexture(terrainHMap->GetTerrainTextureRocks(), 2, "diffuseRocksTex", shader);
	OGLRenderer::BindTexture(terrainHMap->GetTerrainTextureGround(), 3, "diffuseGroundTex", shader);

	terrainHMap->Draw();
}