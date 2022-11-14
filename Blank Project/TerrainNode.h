#pragma once
#include "../nclgl/SceneNode.h"
#include "TerrainHeightmap.h"

class TerrainNode : public SceneNode
{
public:
	TerrainNode();
	~TerrainNode();

	Vector3 GetHeightmapSize() { return terrainHMap->GetHeightMapSize(); }
	Shader* GetTerrainShader() { return terrainHMap->GetTerrainShader(); }

	virtual Mesh* GetMesh() const override { return terrainHMap; }
	virtual void Draw(const OGLRenderer& r) override;

protected:
	TerrainHeightmap* terrainHMap;
};