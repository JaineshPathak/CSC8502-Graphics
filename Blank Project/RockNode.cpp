#include "RockNode.h"

RockNode::RockNode()
{
	nodeName = "Rock";
	bumpTexture = SOIL_load_OGL_texture(TEXTUREDIRCOURSE"Rocks/Rock5_N.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//SetModelScale(Vector3(3.5f, 7.5f, 3.5f));
}

RockNode::RockNode(std::string suffixName)
{
	nodeName = "Rock" + suffixName;
	bumpTexture = SOIL_load_OGL_texture(TEXTUREDIRCOURSE"Rocks/Rock5_N.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//SetModelScale(Vector3(3.5f, 7.5f, 3.5f));
}

void RockNode::Draw(const OGLRenderer& r)
{
	OGLRenderer::BindTexture(texture, 0, "diffuseTex", shader);
	OGLRenderer::BindTexture(bumpTexture, 1, "bumpTex", shader);
	mesh->Draw();
}