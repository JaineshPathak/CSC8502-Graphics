#include "RockNode.h"

RockNode::RockNode()
{
	nodeName = "Rock";
	//SetModelScale(Vector3(3.5f, 7.5f, 3.5f));
}

RockNode::RockNode(std::string suffixName)
{
	nodeName = "Rock" + suffixName;
	//SetModelScale(Vector3(3.5f, 7.5f, 3.5f));
}

void RockNode::Draw(const OGLRenderer& r)
{
	OGLRenderer::BindTexture(texture, 0, "diffuseTex", shader);
	mesh->Draw();
}