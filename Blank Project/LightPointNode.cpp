#include "LightPointNode.h"

LightPointNode::LightPointNode(const Vector4& lightColour, const Vector4& lightSpecularColour, const float& lightRadius)
{
	pointLightColour = lightColour;
	pointLightSpecularColour = lightSpecularColour;
	pointLightRadius = lightRadius;

	light = new Light();
	light->SetColour(lightColour);
	light->SetSpecularColour(lightSpecularColour);
	light->SetRadius(lightRadius);
	light->SetPosition(GetWorldTransform().GetPositionVector());
}

void LightPointNode::Update(float dt)
{
	light->SetPosition(GetWorldTransform().GetPositionVector());
}