#include "Water.h"

Water::Water(const Vector3& pos, const Vector3& rot, const Vector3& scale, GLuint waterTex, Mesh* waterMesh)
{
	SetModelRotation(rot);
	SetModelScale(scale);

	waterCubeTex = waterTex;

	colour = Vector4(1, 1, 1, 1);
	mesh = waterMesh;
	texture = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	OGLRenderer::SetTextureRepeating(texture, true);
	
	shader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	SetTransform(Matrix4::Translation(pos) * GetRotationMatrix() * Matrix4::Scale(GetModelScale()));
}

Water::~Water()
{
	glDeleteTextures(1, &waterCubeTex);
}

void Water::Draw(const OGLRenderer& r)
{
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "waterTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "cubeTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, waterCubeTex);

	mesh->Draw();
}