#include "TreePropNode.h"

TreePropNode::TreePropNode(Mesh* m, MeshMaterial* treeMat, Shader* s, const std::string& texPath)
{
	mesh = m;
	shader = s;
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		const MeshMaterialEntry* matEntry = treeMat->GetMaterialForLayer(i);

		const string* fileName = nullptr;
		matEntry->GetEntry("Diffuse", &fileName);
		string path = texPath + *fileName;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		OGLRenderer::SetTextureRepeating(texID, true);
		matTextures.emplace_back(texID);

		const string* bumpFileName = nullptr;
		matEntry->GetEntry("Bump", &bumpFileName);
		string bumpPath = texPath + *bumpFileName;
		GLuint bumptexID = SOIL_load_OGL_texture(bumpPath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		OGLRenderer::SetTextureRepeating(bumptexID, true);
		matBumpTextures.emplace_back(bumptexID);
	}
}

void TreePropNode::Draw(const OGLRenderer& r)
{
	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		OGLRenderer::BindTexture(matTextures[i], 0, "diffuseTex", shader);
		OGLRenderer::BindTexture(matBumpTextures[i], 1, "bumpTex", shader);

		mesh->DrawSubMesh(i);
	}
}