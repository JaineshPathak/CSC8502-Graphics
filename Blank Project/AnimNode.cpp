#include "AnimNode.h"

AnimMeshNode::AnimMeshNode(Shader* shader, Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat)
{
	this->mesh = mesh;
	this->shader = shader;
	this->MeshAnim = anim;
	this->MeshMat = mat;

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i)
	{
		const MeshMaterialEntry* matEntry = MeshMat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
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
	if (mesh) { mesh->Draw(); }
}

void AnimMeshNode::AnimateMesh()
{
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	//UpdateShaderMatrices();
	vector <Matrix4 > frameMatrices;
	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = MeshAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i)
	{
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
}

void AnimMeshNode::DrawMaterial()
{
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		mesh->DrawSubMesh(i);
	}
}
