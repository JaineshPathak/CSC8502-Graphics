#pragma once

#include <nclgl/SceneNode.h>
#include "TerrainHeightmap.h"
#include <memory>

class LightPointNode;
class DirectionalLight;
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
	void DrawGrass();

protected:
	std::shared_ptr<TerrainHeightmap> m_TerrainHMap;

	std::shared_ptr<DirectionalLight> m_DirLight;
	std::vector<std::shared_ptr<LightPointNode>> m_PointLightsList;

	//Grasses Data
	unsigned int m_GrassInstancedVBO;
	unsigned int m_GrassTexID;
	unsigned int m_GrassWindTexID;
	std::shared_ptr<Mesh> m_GrassPointMesh;
	std::shared_ptr<Shader> m_GrassShader;
	std::shared_ptr<Shader> m_NormalsShader;
	std::vector<Matrix4> m_GrassModelMats;
};