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

	GLuint GetTerrainTextureSplatmap() { return m_TerrainTextureSplatmap; }
	GLuint GetTerrainTextureGrass() { return m_TerrainTextureGrass; }
	GLuint GetTerrainTextureGrassBump() { return m_TerrainTextureGrassBump; }
	GLuint GetTerrainTextureRocks() { return m_TerrainTextureRocks; }
	GLuint GetTerrainTextureRocksBump() { return m_TerrainTextureRocksBump; }
	GLuint GetTerrainTextureGround() { return m_TerrainTextureGround; }
	GLuint GetTerrainTextureGroundBump() { return m_TerrainTextureGroundBump; }

	bool InitSuccess() { return m_InitSuccess; }

protected:
	bool m_InitSuccess;

	std::shared_ptr<Shader> m_TerrainShader;
	GLuint m_TerrainTextureSplatmap;
	GLuint m_TerrainTextureGrass;
	GLuint m_TerrainTextureGrassBump;
	GLuint m_TerrainTextureRocks;
	GLuint m_TerrainTextureRocksBump;
	GLuint m_TerrainTextureGround;
	GLuint m_TerrainTextureGroundBump;
};