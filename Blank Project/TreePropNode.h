#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshMaterial.h"

class TreePropNode : public SceneNode
{
public:
	TreePropNode(Mesh* m, MeshMaterial* treeMat, Shader* s, const std::string& texPath);
	virtual void Draw(const OGLRenderer& r) override;

protected:
	std::vector<GLuint> matTextures;
};