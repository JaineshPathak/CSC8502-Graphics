#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

class AnimMeshNode : public SceneNode
{
public:
	AnimMeshNode(Shader* shader, Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat, const std::string& texPath);
	~AnimMeshNode();

	virtual void Update(float dt) override;
	virtual void Draw(const OGLRenderer& r) override;

	//void AnimateMesh();
	//void DrawMaterial();

protected:
	MeshAnimation* MeshAnim;
	MeshMaterial* MeshMat;
	vector<GLuint> matTextures;
	vector<GLuint> matBumpTextures;
	int currentFrame;
	float frameTime;
};