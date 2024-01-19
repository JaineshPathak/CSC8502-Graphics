#pragma once
#include "../nclgl/SceneNode.h"
#include "TerrainHeightmap.h"
#include <memory>

class TerrainNode : public SceneNode
{
public:
	TerrainNode();
	~TerrainNode() {};

	Vector3 GetHeightmapSize() { return m_TerrainHMap->GetHeightMapSize(); }
	Shader* GetTerrainShader() { return m_TerrainHMap->GetTerrainShader(); }

	virtual Mesh* GetMesh() const override { return m_TerrainHMap.get(); }
	virtual void Draw(const OGLRenderer& r) override;
	virtual void DepthDraw(Shader* s) override;

protected:
	std::shared_ptr<TerrainHeightmap> m_TerrainHMap;
};