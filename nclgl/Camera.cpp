#include "Camera.h"
#include "Window.h"
#include <algorithm>
#include <imgui/imgui.h>

void Camera::UpdateCamera(float dt)
{
	if (ImGui::GetCurrentContext() && ImGui::GetIO().MouseDrawCursor)
		return;

	camRotation.x -= (Window::GetMouse()->GetRelativePosition().y) * 2.0f;
	camRotation.y -= (Window::GetMouse()->GetRelativePosition().x) * 2.0f;

	camRotation.x = std::min(camRotation.x, 90.0f);
	camRotation.x = std::max(camRotation.x, -90.0f);

	if (camRotation.y < 0)
		camRotation.y += 360.0f;

	if (camRotation.y > 360.0f)
		camRotation.y -= 360.0f;

	Matrix4 rotation = Matrix4::Rotation(camRotation.y, Vector3(0, 1, 0)) * Matrix4::Rotation(camRotation.x, Vector3(1, 0, 0));
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	/*camViewMat = Matrix4::BuildViewMatrix(camPosition, camPosition + camFront, camUp);

	Vector3 lookDir;
	lookDir.x = cos(DegToRad(camRotation.y)) * cos(DegToRad(camRotation.x));
	lookDir.y = sin(DegToRad(camRotation.x));
	lookDir.z = sin(DegToRad(camRotation.y)) * cos(DegToRad(camRotation.x));
	camFront = lookDir.Normalised();
	camRight = Vector3::Cross(camFront, camUp).Normalised();*/

	currentSpeed = defaultSpeed * dt;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))
		currentSpeed *= 2.0f;
	
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))
		camPosition += forward * currentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))
		camPosition -= forward * currentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))
		camPosition -= right * currentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))
		camPosition += right * currentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))
		camPosition.y += currentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL))
		camPosition.y -= currentSpeed;

	/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))
		camPosition.y += speed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))
		camPosition.y -= speed;*/
}

void Camera::SetProjMatrix(const float& znear, const float& zfar, const float& width, const float& height, const float& fov)
{
	const float aspect = width / height;
	camProjMat = Matrix4::Perspective(znear, zfar, aspect, fov);
}

Matrix4 Camera::BuildViewMatrix()
{
	camViewMat = Matrix4::Rotation(-camRotation.x, Vector3(1, 0, 0)) * Matrix4::Rotation(-camRotation.y, Vector3(0, 1, 0)) * Matrix4::Translation(-camPosition);
	return camViewMat;
	//return Matrix4::Rotation(-camRotation.x, Vector3(1, 0, 0)) * Matrix4::Rotation(-camRotation.y, Vector3(0, 1, 0)) * camViewMat;
	//return camViewMat;
}