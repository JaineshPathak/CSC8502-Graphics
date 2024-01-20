#include "LightPointNode.h"

LightPointNode::LightPointNode()
{
	m_PointLightColour = Vector4();
	m_PointLightSpecularColour = Vector4();
	m_PointLightRadius = 1.0f;
	m_PointLightIntensity = 1.0f;

	//m_PointLight = new Light();
	m_PointLight.SetColour(m_PointLightColour);
	m_PointLight.SetSpecularColour(m_PointLightSpecularColour);
	m_PointLight.SetRadius(m_PointLightRadius);
	m_PointLight.SetIntensity(m_PointLightIntensity);
	m_PointLight.SetPosition(GetWorldTransform().GetPositionVector());
}

LightPointNode::LightPointNode(const Vector4& lightColour, const Vector4& lightSpecularColour, const float& lightRadius, const float& lightIntensity)
{
	m_PointLightColour = lightColour;
	m_PointLightSpecularColour = lightSpecularColour;
	m_PointLightRadius = lightRadius;
	m_PointLightIntensity = lightIntensity;

	//m_PointLight = new Light();
	m_PointLight.SetColour(lightColour);
	m_PointLight.SetSpecularColour(lightSpecularColour);
	m_PointLight.SetRadius(lightRadius);
	m_PointLight.SetIntensity(lightIntensity);
	m_PointLight.SetPosition(GetWorldTransform().GetPositionVector());
}

LightPointNode::~LightPointNode()
{
	//delete m_PointLight;
}

Vector3 LightPointNode::GetLightPosition() const
{
	return m_PointLight.GetPosition();
}

void LightPointNode::SetPosition(const Vector3& pos)
{
	modelPosition = pos;
	SetTransform(Matrix4::Translation(modelPosition));

	m_PointLight.SetPosition(pos);
}

void LightPointNode::SetLightRadius(const float& radius)
{
	m_PointLightRadius = radius;

	m_PointLight.SetRadius(radius);
}

void LightPointNode::SetLightIntensity(const float& intensity)
{
	m_PointLightIntensity = intensity;

	m_PointLight.SetIntensity(intensity);
}

void LightPointNode::SetLightColour(const Vector4& lightColour)
{
	m_PointLightColour = lightColour;

	m_PointLight.SetColour(lightColour);
}

void LightPointNode::SetLightSpecularColour(const Vector4& lightSpecularColour)
{
	m_PointLightSpecularColour = lightSpecularColour;

	m_PointLight.SetSpecularColour(lightSpecularColour);
}

void LightPointNode::Update(float dt)
{
	//SetPosition(modelPosition);
	SceneNode::Update(dt);

	m_PointLight.SetPosition(GetWorldTransform().GetPositionVector());
}