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
	}
}

void TreePropNode::Draw(const OGLRenderer& r)
{
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		mesh->DrawSubMesh(i);
	}
}