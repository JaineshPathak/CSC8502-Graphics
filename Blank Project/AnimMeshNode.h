#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

class AnimMeshNode : public SceneNode
{
public:
	AnimMeshNode(Shader* shader, Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat, const std::string& texPath);
	~AnimMeshNode();

	void CalcFrameMatrices();

	virtual void Update(float dt) override;
	virtual void Draw(const OGLRenderer& r) override;
	virtual void DepthDraw(Shader* s) override;

	vector<Matrix4> GetFrameMatrices() { return frameMatrices; }

	//void AnimateMesh();
	//void DrawMaterial();

protected:
	MeshAnimation* MeshAnim;
	MeshMaterial* MeshMat;
	vector<GLuint> matTextures;
	vector<GLuint> matBumpTextures;
	vector<Matrix4> frameMatrices;
	int currentFrame;
	float frameTime;
};