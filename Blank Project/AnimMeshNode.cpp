#include "AnimMeshNode.h"

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
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);

		const string* bumpFileName = nullptr;
		matEntry->GetEntry("Bump", &bumpFileName);
		if (bumpFileName != nullptr)
		{
			string bumpPath = texPath + *bumpFileName;
			GLuint bumptexID = SOIL_load_OGL_texture(bumpPath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			matBumpTextures.emplace_back(bumptexID);
		}
	}
	currentFrame = 0;
	frameTime = 0.0f;
}

AnimMeshNode::~AnimMeshNode()
{
	delete mesh;
	delete MeshAnim;
	delete MeshMat;
}

void AnimMeshNode::Update(float dt)
{
	frameTime -= dt;
	while (frameTime < 0.0f)
	{
		currentFrame = (currentFrame + 1) % MeshAnim->GetFrameCount();
		frameTime += 1.0f / MeshAnim->GetFrameRate();
	}

	SceneNode::Update(dt);
}

void AnimMeshNode::Draw(const OGLRenderer& r)
{
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
	
	vector <Matrix4> frameMatrices;
	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = MeshAnim->GetJointData(currentFrame);

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

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);

		if (matBumpTextures.size() > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, matBumpTextures[i]);
		}

		mesh->DrawSubMesh(i);
	}
}

//void AnimMeshNode::AnimateMesh()
//{
//	
//}
//
//void AnimMeshNode::DrawMaterial()
//{
//	
//}
