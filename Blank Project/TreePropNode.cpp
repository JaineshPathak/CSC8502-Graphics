#include "TreePropNode.h"
#include "AssetManager.h"
#include "SceneRenderer.h"

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
		//GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		GLuint texID = AssetManager::Get()->GetTexture(*fileName, path);
		OGLRenderer::SetTextureRepeating(texID, true);
		matTextures.emplace_back(texID);

		const string* bumpFileName = nullptr;
		matEntry->GetEntry("Bump", &bumpFileName);
		if (bumpFileName != nullptr)
		{
			string bumpPath = texPath + *bumpFileName;
			//GLuint bumptexID = SOIL_load_OGL_texture(bumpPath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			GLuint bumptexID = AssetManager::Get()->GetTexture(*bumpFileName, bumpPath);
			OGLRenderer::SetTextureRepeating(bumptexID, true);
			matBumpTextures.emplace_back(bumptexID);
		}
	}
}

void TreePropNode::Draw(const OGLRenderer& r)
{
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		//OGLRenderer::BindTexture(matTextures[i], 0, "diffuseTex", shader);
		//OGLRenderer::BindTexture(matBumpTextures[i], 1, "bumpTex", shader);
		shader->SetTexture("diffuseTex", matTextures[i], 0);
		
		if (matBumpTextures.size() > 0)
		{
			if (matBumpTextures[i] != -1)
			{
				shader->SetBool("hasBumpTex", true);
				shader->SetTexture("bumpTex", matBumpTextures[i], 1);
			}
		}

		shader->SetTexture("shadowTex", SceneRenderer::Get()->GetDepthTexture(), 2);

		mesh->DrawSubMesh(i);
	}
}

void TreePropNode::DepthDraw(Shader* s)
{
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		if(matTextures[i] != -1)
			s->SetTexture("diffuseTex", matTextures[i], 0);

		mesh->DrawSubMesh(i);
	}
}
