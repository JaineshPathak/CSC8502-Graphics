#include "TerrainNode.h"
#include "SceneRenderer.h"

#include <nclgl/Camera.h>

const std::size_t VEC4Size = sizeof(Vector4);
const std::size_t MAT4Size = sizeof(Matrix4);

TerrainNode::TerrainNode()
{
	nodeName = "TerrainMain";
	m_TerrainHMap = std::shared_ptr<TerrainHeightmap>(new TerrainHeightmap(TEXTUREDIRCOURSETERRAIN"Terrain_heightmap4.png", 32.0f, 32.0f, 16.0f, 16.0f));
	shader = m_TerrainHMap->GetTerrainShader();

	m_GrassPointMesh = SceneRenderer::Get()->GetPointMesh();
	m_GrassShader = std::shared_ptr<Shader>(new Shader(SHADERDIRCOURSETERRAIN"CWGrassVertex.glsl", SHADERDIRCOURSETERRAIN"CWGrassFragment.glsl", SHADERDIRCOURSETERRAIN"CWGrassGeometry.glsl"));

	//Creating in Instance VBO
	glGenBuffers(1, &m_GrassInstancedVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_GrassInstancedVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * m_TerrainHMap->GetVerticesCount(), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Setting up the Vertex Positions in World Space for Grasses
	m_GrassModelMats.resize(m_TerrainHMap->GetVerticesCount());
	UpdateGrassData();
}

void TerrainNode::Draw(const OGLRenderer& r)
{
	shader->SetBool("hasBumpTex", true);
	shader->SetTexture("diffuseSplatmapTex", m_TerrainHMap->GetTerrainTextureSplatmap(), 0);
	shader->SetTexture("diffuseGrassTex", m_TerrainHMap->GetTerrainTextureGrass(), 1);
	shader->SetTexture("bumpGrassTex", m_TerrainHMap->GetTerrainTextureGrassBump(), 2);
	shader->SetTexture("diffuseRocksTex", m_TerrainHMap->GetTerrainTextureRocks(), 3);
	shader->SetTexture("bumpRocksTex", m_TerrainHMap->GetTerrainTextureRocksBump(), 4);
	shader->SetTexture("diffuseGroundTex", m_TerrainHMap->GetTerrainTextureGround(), 5);
	shader->SetTexture("bumpGroundTex", m_TerrainHMap->GetTerrainTextureGroundBump(), 6);
	shader->SetTexture("shadowTex", SceneRenderer::Get()->GetDepthTexture(), 7);

	m_TerrainHMap->Draw();

	//Grasses Points Drawing
	m_GrassShader->Bind();

	Matrix4 viewMatrix = SceneRenderer::Get()->GetCamera()->GetViewMatrix();
	Matrix4 projMatrix = SceneRenderer::Get()->GetCamera()->GetProjMatrix();

	m_GrassShader->SetMat4("viewMatrix", viewMatrix);
	m_GrassShader->SetMat4("projMatrix", projMatrix);

	m_GrassPointMesh->DrawInstanced((int)m_GrassModelMats.size());

	m_GrassShader->UnBind();
}

void TerrainNode::DepthDraw(Shader* s)
{
	m_TerrainHMap->Draw();
}

void TerrainNode::UpdateGrassData()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_GrassInstancedVBO);

	for (size_t i = 0; i < m_GrassModelMats.size(); i++)
		m_GrassModelMats[i] = localTransform * Matrix4::Translation(m_TerrainHMap->GetVertices()[i]) * Matrix4::Scale(10.0f);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * m_TerrainHMap->GetVerticesCount(), &m_GrassModelMats[0], GL_STATIC_DRAW);

	glBindVertexArray(m_GrassPointMesh->GetVertexArrayObject());
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, MAT4Size, (void*)0);
	glEnableVertexAttribArray(10);
	glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, MAT4Size, (void*)(1 * VEC4Size));
	glEnableVertexAttribArray(11);
	glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, MAT4Size, (void*)(2 * VEC4Size));
	glEnableVertexAttribArray(12);
	glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, MAT4Size, (void*)(3 * VEC4Size));

	glVertexAttribDivisor(9, 1);
	glVertexAttribDivisor(10, 1);
	glVertexAttribDivisor(11, 1);
	glVertexAttribDivisor(12, 1);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
