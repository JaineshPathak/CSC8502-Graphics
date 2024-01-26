#pragma once

#include <nclgl/SceneNode.h>
#include "TerrainHeightmap.h"
#include <memory>

class TerrainNode : public SceneNode
{
public:
	TerrainNode();
	~TerrainNode() {};

	Vector3 GetHeightmapSize() { return m_TerrainHMap->GetHeightMapSize(); }
	Shader* GetTerrainShader() { return m_TerrainHMap->GetTerrainShader(); }

	virtual void SetTransform(const Matrix4& matrix) override { localTransform = matrix; UpdateGrassData(); };
	virtual Mesh* GetMesh() const override { return m_TerrainHMap.get(); }

	virtual void Draw(const OGLRenderer& r) override;
	virtual void DepthDraw(Shader* s) override;

private:
	void UpdateGrassData();

protected:
	std::shared_ptr<TerrainHeightmap> m_TerrainHMap;

	//Grasses Data
	unsigned int m_GrassInstancedVBO;
	std::shared_ptr<Mesh> m_GrassPointMesh;
	std::shared_ptr<Shader> m_GrassShader;
	std::vector<Matrix4> m_GrassModelMats;
};