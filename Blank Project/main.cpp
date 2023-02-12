#include "Renderer.h"
#include "../NCLGL/window.h"
#include "../Third Party/imgui/imgui_internal.h"

int main()	
{
	Window w("CSC8502 Coursework - Jainesh Pathak!", 1280, 768, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	bool showPointer = true;
	w.LockMouseToWindow(showPointer);
	w.ShowOSPointer(!showPointer);
	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE))
	{
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) 
		{
			Shader::ReloadAllShaders();
		}

		/*if (Window::GetMouse()->ButtonDown(MouseButtons::MOUSE_RIGHT))
		{
			showPointer = !showPointer;
			w.ShowOSPointer(showPointer);
		}*/

		if (ImGui::GetIO().MouseClicked[1])
		{
			showPointer = !showPointer;
			ImGui::GetIO().MouseDrawCursor = showPointer;
			w.LockMouseToWindow(!showPointer);
			/*w.LockMouseToWindow(showPointer);
			w.ShowOSPointer(!showPointer);*/
		}
	}
	return 0;
}