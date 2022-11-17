#pragma once

#include "Vector4.h"
#include "Vector3.h"

enum LIGHT_TYPE
{
	TYPE_POINTLIGHT = 0,
	TYPE_DIRECTIONALLIGHT = 1
};

class Light
{
public:
	Light() {}
	Light(const Vector3& position, const Vector4& colour, float radius)
	{
		this->position = position;
		this->colour = colour;
		this->specularColour = Vector4();
		this->radius = radius;
	}

	Light(const Vector3& position, const Vector4& colour, const Vector4& specularColour, float radius)
	{
		this->position = position;
		this->colour = colour;
		this->specularColour = specularColour;
		this->radius = radius;
	}

	~Light(void) {}

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

	Vector4 GetSpecularColour() const { return specularColour; }
	void SetSpecularColour(const Vector4& val) { specularColour = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	float GetIntensity() const { return intensity; }
	void SetIntensity(float val) { intensity = val; }

protected:
	LIGHT_TYPE lightType = TYPE_POINTLIGHT;
	Vector3 position;
	Vector4 colour;
	Vector4 specularColour;
	float radius;
	float intensity = 1.0f;
};