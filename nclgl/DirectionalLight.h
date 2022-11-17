#pragma once

#include "Light.h"
#include "Vector3.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight(const Vector3& lightDir, const Vector4& lightColour, const Vector4& lightSpecularColour)
	{
		lightType = TYPE_DIRECTIONALLIGHT;
		
		this->lightDir = lightDir;
		this->colour = lightColour;
		this->specularColour = lightSpecularColour;
		
		this->position = Vector3(0, 0, 0);
		this->radius = 0;
	}

	inline Vector3 GetLightDir() const { return lightDir; }
	inline void SetLightDir(const Vector3& dir) { lightDir = dir; }

protected:
	Vector3 lightDir;
};