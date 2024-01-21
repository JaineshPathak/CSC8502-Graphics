#pragma once
#include <nclgl/SceneNode.h>
#include <memory>

class AssetManager;
class WaterPropNode : public SceneNode
{
public:
	WaterPropNode();
	~WaterPropNode();

	void SetSkyboxTexID(const unsigned int& skyboxTexID) { m_SkyboxTexID = skyboxTexID; }

	virtual void Draw(const OGLRenderer& r) override;
	virtual void Update(float DeltaTime) override;
	virtual void DepthDraw(Shader* s) override;

private:
	unsigned int m_WaterTexID;
	unsigned int m_WaterBumpTexID;
	unsigned int m_SkyboxTexID;

	float m_WaterPanSpeed;

	AssetManager* m_AssetManager;
	Matrix4 m_WaterTexMatrix;
};