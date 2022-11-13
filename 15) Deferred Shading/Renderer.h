#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Mesh;
class HeightMap;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float dt) override;
	void RenderScene();

protected:
	void DrawShadowScene();
	void StepOneFillBuffers();
	void StepTwoDrawPointLights();
	void StepThreeCombineBuffers();

	void GenerateScreenTexture(GLuint& into, bool depth = false);

	//----------------------------
	GLuint shadowFBO;
	GLuint shadowTex;
	Matrix4 shadowMat;
	//----------------------------

	Shader* sceneShader;
	Shader* pointLightShader;
	Shader* combineShader;
	Shader* shadowShader;

	GLuint bufferFBO;
	GLuint bufferColourTex;
	GLuint bufferNormalTex;
	GLuint bufferDepthTex;

	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

	HeightMap* heightMap;			//terrain
	Light* pointLights;				//Array of light data
	Mesh* sphere;					//Light Volume
	Mesh* quad;
	Camera* camera;
	GLuint earthTex;
	GLuint earthBump;

	GameTimer* timer;
};