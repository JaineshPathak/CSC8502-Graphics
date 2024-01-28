#pragma once
#include "../nclgl/HeightMap.h"
#include <memory>

class Shader;
class TerrainHeightmap : public HeightMap
{
public:
	TerrainHeightmap(const std::string& name, 
					float vertexScaleWidth = 16.0f, 
					float vertexScaleLength = 16.0f, 
					float texturingScale = 16.0f,
					float extraHeight = 1.0f);

	~TerrainHeightmap();

	Shader* GetTerrainShader() { return m_TerrainShader.get(); }

	unsigned int GetTerrainTextureSplatmap() { return m_TerrainTextureSplatmap; }
	unsigned int GetTerrainTextureGrass() { return m_TerrainTextureGrass; }
	unsigned int GetTerrainTextureGrassBump() { return m_TerrainTextureGrassBump; }
	unsigned int GetTerrainTextureRocks() { return m_TerrainTextureRocks; }
	unsigned int GetTerrainTextureRocksBump() { return m_TerrainTextureRocksBump; }
	unsigned int GetTerrainTextureGround() { return m_TerrainTextureGround; }
	unsigned int GetTerrainTextureGroundBump() { return m_TerrainTextureGroundBump; }

	bool InitSuccess() { return m_InitSuccess; }

protected:
	bool m_InitSuccess;

	std::shared_ptr<Shader> m_TerrainShader;

	unsigned int m_TerrainTextureSplatmap;
	unsigned int m_TerrainTextureGrass;
	unsigned int m_TerrainTextureGrassBump;
	unsigned int m_TerrainTextureRocks;
	unsigned int m_TerrainTextureRocksBump;
	unsigned int m_TerrainTextureGround;
	unsigned int m_TerrainTextureGroundBump;
};