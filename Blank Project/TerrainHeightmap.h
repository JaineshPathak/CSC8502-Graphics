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

	Shader* GetTerrainShader() { return terrainShader.get(); }
	GLuint GetTerrainTextureSplatmap() { return terrainTextureSplatmap; }
	GLuint GetTerrainTextureGrass() { return terrainTextureGrass; }
	GLuint GetTerrainTextureRocks() { return terrainTextureRocks; }
	GLuint GetTerrainTextureGround() { return terrainTextureGround; }

	bool InitSuccess() { return initSuccess; }

protected:
	bool initSuccess = true;

	std::shared_ptr<Shader> terrainShader;
	GLuint terrainTextureSplatmap;
	GLuint terrainTextureGrass;
	GLuint terrainTextureRocks;
	GLuint terrainTextureGround;
};