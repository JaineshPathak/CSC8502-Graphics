#pragma once
#include <nclgl/SceneNode.h>

class LightPointNode : public SceneNode
{
public:
	LightPointNode();
	LightPointNode(const Vector4& lightColour, const Vector4& lightSpecularColour, const float& lightRadius, const float& lightIntensity);
	~LightPointNode();

	Vector3 GetLightPosition() const;
	void SetPosition(const Vector3& pos);

	float GetLightRadius() const { return m_PointLightRadius; }
	void SetLightRadius(const float& radius) { m_PointLightRadius = radius; }

	float GetLightIntensity() const { return m_PointLightIntensity; }
	void SetLightIntensity(const float& intensity) { m_PointLightIntensity = intensity; }

	Vector4 GetLightColour() const { return m_PointLightColour; }
	void SetLightColour(const Vector4& lightColour) { m_PointLightColour = lightColour; }

	Vector4 GetLightSpecularColour() const { return m_PointLightColour; }
	void SetLightSpecularColour(const Vector4& lightSpecularColour) { m_PointLightSpecularColour = lightSpecularColour; }

protected:
	float m_PointLightRadius;
	float m_PointLightIntensity;
	Vector4 m_PointLightColour;
	Vector4 m_PointLightSpecularColour;
};