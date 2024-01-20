#include "LightPointNode.h"

LightPointNode::LightPointNode()
{
	m_PointLightColour = Vector4();
	m_PointLightSpecularColour = Vector4();
	m_PointLightRadius = 1.0f;
	m_PointLightIntensity = 1.0f;
}

LightPointNode::LightPointNode(const Vector4& lightColour, const Vector4& lightSpecularColour, const float& lightRadius, const float& lightIntensity)
{
	m_PointLightColour = lightColour;
	m_PointLightSpecularColour = lightSpecularColour;
	m_PointLightRadius = lightRadius;
	m_PointLightIntensity = lightIntensity;
}

LightPointNode::~LightPointNode()
{
	//delete m_PointLight;
}

Vector3 LightPointNode::GetLightPosition() const
{
	return GetWorldTransform().GetPositionVector();
}

void LightPointNode::SetPosition(const Vector3& pos)
{
	modelPosition = pos;
	SetTransform(Matrix4::Translation(modelPosition));
}

void LightPointNode::SetLightRadius(const float& radius)
{
	m_PointLightRadius = radius;
}

void LightPointNode::SetLightIntensity(const float& intensity)
{
	m_PointLightIntensity = intensity;
}

void LightPointNode::SetLightColour(const Vector4& lightColour)
{
	m_PointLightColour = lightColour;
}

void LightPointNode::SetLightSpecularColour(const Vector4& lightSpecularColour)
{
	m_PointLightSpecularColour = lightSpecularColour;
}