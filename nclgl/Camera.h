#pragma once
#include "Matrix4.h"
#include "Vector3.h"

class Camera
{
public:
	Camera(void)
	{
		//yaw = 0.0f;
		//pitch = 0.0;
		camPosition.ToZero();

		camRotation.x = 0.0f;
		camRotation.y = 0.0f;
		camRotation.z = 0.0f;

		camFront = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
	}

	Camera(float _pitch, float _yaw, float _roll, Vector3 _position)
	{
		//yaw = _yaw;
		//pitch = _pitch;
		camPosition = _position;

		camRotation.x = _pitch;
		camRotation.y = _yaw;
		camRotation.z = _roll;

		camFront = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
	}

	Camera(Vector3 _position, Vector3 _rotation)
	{
		//yaw = _yaw;
		//pitch = _pitch;
		camPosition = _position;
		camRotation = _rotation;

		camFront = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
	}

	~Camera(void) {}

	float getDefaultSpeed() const { return defaultSpeed; }
	inline void SetDefaultSpeed(float s) { defaultSpeed = s; }

	void UpdateCamera(float dt = 1.0f);
	Matrix4 BuildViewMatrix();

	Vector3 getPosition() const { return camPosition; }
	void SetPosition(const Vector3& val) { camPosition = val; }

	Vector3 getRotation() const { return camRotation; }
	void SetRotation(const Vector3& val) { camRotation = val; }

	float GetPitch() const { return camRotation.x; }
	void SetPitch(float p) { camRotation.x = p; }

	float GetYaw() const { return camRotation.y; }
	void SetYaw(float val) { camRotation.y = val; }

	float GetRoll() const { return camRotation.z; }
	void SetRoll(float val) { camRotation.z = val; }


protected:
	//float pitch;
	//float yaw;
	float currentSpeed;
	float defaultSpeed = 30.0f;

	Vector3 camPosition;
	Vector3 camRotation;

	Vector3 camFront;
	Vector3 camUp;
	Vector3 camRight;

	Matrix4 camViewMat;
};