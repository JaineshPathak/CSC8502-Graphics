#include "../nclgl/window.h"
#include "Renderer.h"

int main() {	
	Window w("Scene Graphs!", 1280, 720,false);
	if(!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}	

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE))
	{
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) 
			Shader::ReloadAllShaders();
		
		float s = 1;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		{
			s += 0.01f;
			renderer.UpdateRobotScale(s);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		{
			s -= 0.01f;
			renderer.UpdateRobotScale(s);
		}
		
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
	}

	return 0;
}