#pragma once
#include "..\nclgl\SceneNode.h"
#include "..\nclgl\Light.h"

class LightPointNode : public SceneNode
{
public:
	LightPointNode(const Vector4& lightColour, const Vector4& lightSpecularColour, const float& lightRadius);

	int GetLightType() { return lightType; }

	virtual Vector4 GetLightColour() { return pointLightColour; }
	virtual Vector4 GetLightSpecularColour() { return pointLightColour; }
	virtual float GetLightRadius() { return pointLightRadius; }

	virtual void Update(float dt) override;

protected:
	Light* light;
	LIGHT_TYPE lightType = TYPE_POINTLIGHT;

	//Point Lights Properties
	float pointLightRadius;
	Vector4 pointLightColour;
	Vector4 pointLightSpecularColour;
};