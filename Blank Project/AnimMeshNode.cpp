#include "AnimMeshNode.h"
#include "AssetManager.h"
#include "SceneRenderer.h"

#include <nclgl/Camera.h>

AnimMeshNode::AnimMeshNode(Shader* shader, Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat, const std::string& texPath)
{
	this->shader = shader;
	this->mesh = mesh;
	this->MeshAnim = anim;
	this->MeshMat = mat;

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i)
	{
		const MeshMaterialEntry* matEntry = MeshMat->GetMaterialForLayer(i);
		const string* fileName = nullptr;
		matEntry->GetEntry("Diffuse", &fileName);
		string path = texPath + *fileName;
		//GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		GLuint texID = AssetManager::Get()->GetTexture(*fileName, path);
		matTextures.emplace_back(texID);

		const string* bumpFileName = nullptr;
		matEntry->GetEntry("Bump", &bumpFileName);
		if (bumpFileName != nullptr)
		{
			string bumpPath = texPath + *bumpFileName;
			//GLuint bumptexID = SOIL_load_OGL_texture(bumpPath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			GLuint bumptexID = AssetManager::Get()->GetTexture(*bumpFileName, bumpPath);
			matBumpTextures.emplace_back(bumptexID);
		}
	}
	currentFrame = 0;
	frameTime = 0.0f;
}

AnimMeshNode::~AnimMeshNode()
{
}

void AnimMeshNode::CalcFrameMatrices()
{
	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = MeshAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i)
	{
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}
}

void AnimMeshNode::Update(float dt)
{
	SceneNode::Update(dt);

	frameTime -= dt;
	while (frameTime < 0.0f)
	{
		currentFrame = (currentFrame + 1) % MeshAnim->GetFrameCount();
		frameTime += 1.0f / MeshAnim->GetFrameRate();
	}
}

void AnimMeshNode::Draw(const OGLRenderer& r)
{
	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);

	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = MeshAnim->GetJointData(currentFrame);

	frameMatrices.clear();
	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i)
	{
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, (GLsizei)frameMatrices.size(), false, (float*)frameMatrices.data());
	
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i)
	{
		//OGLRenderer::BindTexture(matTextures[i], 0, "diffuseTex", shader);
		//OGLRenderer::BindTexture(matBumpTextures[i], 1, "bumpTex", shader);

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);

		if (matBumpTextures.size() > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, matBumpTextures[i]);
		}*/

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

	frameMatrices.clear();
}

void AnimMeshNode::DepthDraw(Shader* s)
{
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		if (matTextures[i] != -1)
			s->SetTexture("diffuseTex", matTextures[i], 0);

		mesh->DrawSubMesh(i);
	}
}
