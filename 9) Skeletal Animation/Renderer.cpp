#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

Renderer::Renderer(Window& parent) : w(parent), OGLRenderer(parent)
{
	//-----------------------------------------------------------
	//Imgui 
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
	//-----------------------------------------------------------

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projMatrix = Matrix4::Perspective(0.01f, 10000.0f, (float)width / (float)height, 45.0f);
	camera = new Camera(-3.0f, 0.0f, 0.0f, Vector3(0, 1.4f, 4.0f));
	camera->SetDefaultSpeed(10.0f);

	shader = new Shader("SkinningVertex.glsl", "TexturedFragmentSkinning.glsl");
	if (!shader->LoadSuccess())
		return;

	mesh = Mesh::LoadFromMeshFile("Character_Boss.msh");
	material = new MeshMaterial("Character_Boss.mat");
	anim = new MeshAnimation("Boss_Gun_Dance.anm");
	anim2 = new MeshAnimation("Boss_Gun_Run.anm");
	jumpAnim = new MeshAnimation("Boss_Gun_Jump.anm");

	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);

		const string* fileName = nullptr;
		matEntry->GetEntry("Diffuse", &fileName);
		string path = TEXTUREDIR + *fileName;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}

	for (size_t i = 0; i < anim->GetFrameCount(); i++)
		framePosList.emplace_back(anim->GetJointData(i)->GetPositionVector());

	currentFrame = 0;
	frameTime = 0.0f;
	playAutomatic = false;

	editMode = false;
	blendFactor = 0.0f;
	init = true;

	currentAnim = anim;
	previousAnim = nullptr;
	pendingAnim = nullptr;
	isTweening = false;
}

Renderer::~Renderer(void)
{
	delete camera;
	delete mesh;
	delete anim;
	delete material;
	delete shader;

	//-----------------------------------------------------------
	//Imgui 
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	//-----------------------------------------------------------
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(shader);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex0"), 0);

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;

	if (editMode)
	{
		/*Matrix4 currentFrameMat = *(anim->GetJointData(currentFrame));
		Matrix4 currentFrameMatInverse = mesh->GetInverseBindPose()[currentFrame];
		finalBlend = LerpMat(currentFrameMat, lastFrame, blendFactor);
		finalBlendInverse = LerpMat(currentFrameMatInverse, lastFrameInverse, blendFactor);*/

		/*Matrix4 anim1lastFrameMat = *(anim->GetJointData(anim->GetFrameCount() - 1));
		Matrix4 anim2FirstFrameMat = *(anim2->GetJointData(0));

		Matrix4 anim1lastFrameMatInv = mesh->GetInverseBindPose()[anim->GetFrameCount() - 1];
		Matrix4 anim2FirstFrameMatInv = mesh->GetInverseBindPose()[0];

		finalBlend = LerpMat(anim1lastFrameMat, anim2FirstFrameMat, blendFactor);
		finalBlendInverse = LerpMat(anim1lastFrameMatInv, anim2FirstFrameMatInv, blendFactor);*/

		const Matrix4* anim1Frame = anim->GetJointData(currentFrame);
		const Matrix4* anim2Frame = anim2->GetJointData(0);

		const Matrix4* invBindPose = mesh->GetInverseBindPose();

		std::vector<Matrix4> finalBlending;
		for (size_t i = 0; i < mesh->GetJointCount(); i++)
			finalBlending.emplace_back(LerpMat(anim1Frame[i], anim2Frame[i], blendFactor));

		for (size_t i = 0; i < mesh->GetJointCount(); i++)
			frameMatrices.emplace_back(finalBlending[i] * invBindPose[i]);
	}
	else
	{
		if (isTweening)
		{
			const Matrix4* anim1Frame = currentAnim->GetJointData(currentFrame);
			const Matrix4* anim2Frame = pendingAnim->GetJointData(0);

			const Matrix4* invBindPose = mesh->GetInverseBindPose();

			std::vector<Matrix4> finalBlending;
			for (size_t i = 0; i < mesh->GetJointCount(); i++)
				finalBlending.emplace_back(LerpMat(anim1Frame[i], anim2Frame[i], blendFactor));

			for (size_t i = 0; i < mesh->GetJointCount(); i++)
				frameMatrices.emplace_back(finalBlending[i] * invBindPose[i]);
		}
		else
		{
			const Matrix4* invBindPose = mesh->GetInverseBindPose();
			const Matrix4* frameData = currentAnim->GetJointData(currentFrame);
	
			for (unsigned int i = 0; i < mesh->GetJointCount(); i++)
				frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
		}
	}

	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		mesh->DrawSubMesh(i);
	}

	RenderImGui();
}

void Renderer::RenderImGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::CollapsingHeader("Animations"))
	{		
		bool editMod = editMode;				
		if (ImGui::Checkbox("Edit Mode", (bool*)&editMod)) editMode = editMod;
		if (editMode)
		{
			float blend = blendFactor;
			if (ImGui::DragFloat("Blend Factor", (float*)&blend, 0.05f, 0.0f, 1.0f)) blendFactor = blend;
		}
		else
		{
			bool playAuto = playAutomatic;
			int curFrame = currentFrame;
			if (ImGui::Checkbox("Play Automatic", (bool*)&playAuto)) playAutomatic = playAuto;
			if (ImGui::DragInt("Current Frame", (int*)&curFrame, 1.0f, 0, anim->GetFrameCount() - 1)) currentFrame = curFrame;
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::UpdateScene(float dt)
{
	globalDt = dt;
	elaspedTime += dt;

	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	if (playAutomatic && !editMode && !isTweening)
	{
		frameTime -= dt;
		while (frameTime < 0.0f)
		{
			currentFrame = (currentFrame + 1) % currentAnim->GetFrameCount();
			frameTime += 1.0f / currentAnim->GetFrameRate();
		}
	}

	if (Window::GetKeyboard()->KeyTriggered(KeyboardKeys::KEYBOARD_1)) PlayAnim(anim);
	if (Window::GetKeyboard()->KeyTriggered(KeyboardKeys::KEYBOARD_2)) PlayAnim(anim2);
	if (Window::GetKeyboard()->KeyTriggered(KeyboardKeys::KEYBOARD_3)) PlayAnim(jumpAnim);

	if (isTweening && pendingAnim != nullptr)
	{
		blendFactor += dt * (1 / tweenTime);		
		std::cout << "Blend Factor: " << blendFactor << std::endl;
		tweenTimeCurrent -= dt;
		if (tweenTimeCurrent <= 0.0f)
		{
			tweenTimeCurrent = 0.0f;
			isTweening = false;
			currentFrame = 0;
			currentAnim = pendingAnim;
			std::cout << "Tweening COMPLETE!\n";
		}
	}

	if (ImGui::GetIO().MouseClicked[1])
	{
		showCursor = !showCursor;
		ImGui::GetIO().MouseDrawCursor = showCursor;
		w.LockMouseToWindow(!showCursor);
	}
}

void Renderer::PlayAnim(MeshAnimation* newAnim)
{
	previousAnim = currentAnim;
	pendingAnim = newAnim;

	TweenAnim(previousAnim, pendingAnim, 0.15f);
}

void Renderer::TweenAnim(MeshAnimation* _previousAnim, MeshAnimation* _nextAnim, float time)
{
	isTweening = true;
	blendFactor = 0.0f;
	tweenTimeCurrent = time;
	tweenTime = time;
}
