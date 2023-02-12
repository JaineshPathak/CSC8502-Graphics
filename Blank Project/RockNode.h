#pragma once
#include "..\nclgl\SceneNode.h"
class RockNode : public SceneNode
{
public:
	RockNode();
	RockNode(std::string suffixName);

	virtual void Draw(const OGLRenderer& r) override;

private:
	GLuint bumpTexture;
};