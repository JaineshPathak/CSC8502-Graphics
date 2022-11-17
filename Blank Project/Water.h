#pragma once
#include "..\nclgl\SceneNode.h"
class Water : public SceneNode
{
public:
	Water(const Vector3& pos, const Vector3& rot, const Vector3& scale, GLuint waterTex, Mesh* waterMesh);
	~Water();

	virtual void Draw(const OGLRenderer& r) override;

	void SetWaterCubeTex(GLuint tex) { waterCubeTex = tex; }

protected:
	GLuint waterCubeTex;
};