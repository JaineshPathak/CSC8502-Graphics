#include "Renderer.h"
#include "SceneRenderer.h"
#include "../NCLGL/window.h"
#include "../Third Party/imgui/imgui_internal.h"

int main()	
{
#if _DEBUG
	Window w("CSC8502 Coursework - Elden Ring Theme (Jainesh Pathak)", 1280, 768, false);
#elif _RELEASE
	Window w("CSC8502 Coursework - Elden Ring Theme (Jainesh Pathak)", 1600, 900, true);
#endif

	if(!w.HasInitialised()) {
		return -1;
	}
	
	SceneRenderer renderer(w);
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

		if (ImGui::GetCurrentContext() && ImGui::GetIO().MouseClicked[1])
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