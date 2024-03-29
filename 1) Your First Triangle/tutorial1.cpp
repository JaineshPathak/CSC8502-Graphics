#include "../NCLGL/window.h"
#include "Renderer.h"

int main()	{
	Window w("My First OpenGL Triangle!", 720, 720, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		//renderer.UpdateScene();
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_R))
			renderer.UpdateScene(0.0f);
	}
	return 0;
}