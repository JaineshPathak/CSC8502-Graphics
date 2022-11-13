#include "../nclgl/window.h"
#include "Renderer.h"

int main() {
	Window w("Vertex Transformation!", 1024, 768, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	float scale = 100.0f;
	float rotation = 0.0f;

	float fov = 45.0f;
	Vector3 position(0, 0, -1500.0f);

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1))
			renderer.SwitchToOrthographic();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2))
			renderer.SwitchToPerspective();

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS))  ++scale;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS)) --scale;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))  ++rotation;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT)) --rotation;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_Z))
			position.y -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q))
			position.y += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
			position.x -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J))
			position.x += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_K))
			position.z -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_I))
			position.z += 1.0f;

		/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_M))
		{
			++fov;
			renderer.SwitchToPerspective();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_N))
		{
			--fov;
			renderer.SwitchToPerspective();
		}*/

		//renderer.SetFOV(fov);
		renderer.SetRotation(rotation);
		renderer.SetScale(scale);
		renderer.SetPosition(position);
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
	}

	return 0;
}