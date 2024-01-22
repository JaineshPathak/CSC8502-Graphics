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

		camForward = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
		camRight = Vector3(1.0f, 0.0f, 0.0f);
	}

	Camera(float _pitch, float _yaw, float _roll, Vector3 _position)
	{
		//yaw = _yaw;
		//pitch = _pitch;
		camPosition = _position;

		camRotation.x = _pitch;
		camRotation.y = _yaw;
		camRotation.z = _roll;

		camForward = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
		camRight = Vector3(1.0f, 0.0f, 0.0f);
	}

	Camera(Vector3 _position, Vector3 _rotation)
	{
		//yaw = _yaw;
		//pitch = _pitch;
		camPosition = _position;
		camRotation = _rotation;

		camForward = Vector3(0.0f, 0.0f, -1.0f);
		camUp = Vector3(0.0f, 1.0f, 0.0f);
		camRight = Vector3(1.0f, 0.0f, 0.0f);
	}

	~Camera(void) {}

	void UpdateCamera(float dt = 1.0f);

	float getDefaultSpeed() const { return defaultSpeed; }
	inline void SetDefaultSpeed(float s) { defaultSpeed = s; }
	
	void SetProjMatrix(const float& znear, const float& zfar, const float& width, const float& height, const float& fov = 60.0f);
	Matrix4 GetProjMatrix() const { return camProjMat; }

	Matrix4 BuildViewMatrix();
	Matrix4 GetViewMatrix() const { return camViewMat; }

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

	float GetFOV() const { return m_FOV; }
	void SetFOV(const float& val);

	float GetNear() const { return m_Near; }
	void SetNear(const float& val);

	float GetFar() const { return m_Far; }
	void SetFar(const float& val);

	float GetScreenWidth() const { return m_ScreenWidth; }
	float GetScreenHeight() const { return m_ScreenHeight; }

	Vector3 GetForward() const { return camForward; }
	Vector3 GetRight() const { return camRight; }
	Vector3 GetUp() const { return camUp; }

	Matrix4 GetRotationMatrix() const { return camRotationMat; }

protected:
	//float pitch;
	//float yaw;
	float currentSpeed;
	float defaultSpeed = 30.0f;

	float m_FOV;
	float m_Near, m_Far;
	float m_ScreenWidth, m_ScreenHeight;

	Vector3 camPosition;
	Vector3 camRotation;

	Vector3 camForward;
	Vector3 camUp;
	Vector3 camRight;

	Matrix4 camProjMat;
	Matrix4 camViewMat;
	Matrix4 camRotationMat;
};