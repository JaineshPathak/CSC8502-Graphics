#include "TerrainNode.h"
#include "SceneRenderer.h"
#include "AssetManager.h"
#include "LightPointNode.h"

#include <nclgl/Camera.h>
#include <nclgl/DirectionalLight.h>

const std::size_t VEC4Size = sizeof(Vector4);
const std::size_t MAT4Size = sizeof(Matrix4);

TerrainNode::TerrainNode()
{
	nodeName = "TerrainMain";
	m_TerrainHMap = std::shared_ptr<TerrainHeightmap>(new TerrainHeightmap(TEXTUREDIRCOURSETERRAIN"Terrain_heightmap5.png", 32.0f, 32.0f, 16.0f, 16.0f));
	shader = m_TerrainHMap->GetTerrainShader();

	m_GrassPointMesh = SceneRenderer::Get()->GetPointMesh();
	m_GrassShader = std::shared_ptr<Shader>(new Shader(SHADERDIRCOURSETERRAIN"CWGrassVertex.glsl", SHADERDIRCOURSETERRAIN"CWGrassFragment.glsl", SHADERDIRCOURSETERRAIN"CWGrassGeometry.glsl"));
	m_NormalsShader = std::shared_ptr<Shader>(new Shader(SHADERDIRCOURSETERRAIN"CWNormalsVertex.glsl", SHADERDIRCOURSETERRAIN"CWNormalsFragment.glsl", SHADERDIRCOURSETERRAIN"CWNormalsGeometry.glsl"));
	m_GrassTexID = AssetManager::Get()->GetTexture("GrassTex", TEXTUREDIRCOURSETERRAIN"Grass2_D.png");
	//Creating in Instance VBO
	glGenBuffers(1, &m_GrassInstancedVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_GrassInstancedVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * m_TerrainHMap->GetVerticesCount(), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Setting up the Vertex Positions in World Space for Grasses
	m_GrassModelMats.resize(m_TerrainHMap->GetVerticesCount());
	
	/*int iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(TEXTUREDIRCOURSETERRAIN"Terrain_Splatmap5.png", &iWidth, &iHeight, &iChans, 3);
	Vector4 RedColour = Vector4(255.0f, 0.0f, 0.0f, 255.0f);
	int RedThreshold = 5;
	if (data)
	{
		for (int z = 0; z < iHeight; ++z)
		{
			for (int x = 0; x < iWidth; ++x)
			{
				int offset = (z * iWidth) + x;
				unsigned char* p = data + (4 * offset);
				if (p)
				{
					Vector4 colour = Vector4(p[0], p[1], p[2], p[3]);
					//std::cout << "At Offset: " << offset << ", Color Data: " << colour.x << ", " << colour.y << ", " << colour.z << ", " << colour.w << std::endl;

					int r = (int)colour.x - 255;
					int g = (int)colour.y - 0;
					int b = (int)colour.z - 0;

					//A Red Color
					if ( (r*r + g*g + b*b) <= (RedThreshold * RedThreshold))
					{
						std::cout << "At Offset: " << offset << ", Color Data: " << colour.x << ", " << colour.y << ", " << colour.z << ", " << colour.w << std::endl;
						m_GrassModelMats.emplace_back(Matrix4::Translation(m_TerrainHMap->GetVertices()[offset] + Vector3(0.0f, 1.5f, 0.0f)) * Matrix4::Scale(150.0f));
					}
				}
			}
		}
	}*/

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

	DrawGrass();
}

void TerrainNode::DepthDraw(Shader* s)
{
	m_TerrainHMap->Draw();
}

void TerrainNode::UpdateGrassData()
{
	if (m_GrassModelMats.size() <= 0) return;

	glBindBuffer(GL_ARRAY_BUFFER, m_GrassInstancedVBO);

	for (size_t i = 0; i < m_GrassModelMats.size(); i++)
	{
		m_GrassModelMats[i] = localTransform * 
							Matrix4::Translation(m_TerrainHMap->GetVertices()[i] + Vector3(0.0f, 1.5f, 0.0f)) *
							/*Matrix4::Rotation(RandF(-90.0f, 90.0f), Vector3(0, 1, 0)) * */
							Matrix4::RotationByDirection(m_TerrainHMap->GetNormals()[i]) * 
							Matrix4::Rotation(RandF(-45.0f, 45.0f), Vector3(0, 1, 0)) *
							Matrix4::Scale(RandF(100.0f, 300.0f));

		//m_GrassModelMats[i] = localTransform * m_GrassModelMats[i];
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * m_GrassModelMats.size(), &m_GrassModelMats[0], GL_STATIC_DRAW);

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

void TerrainNode::DrawGrass()
{
	//Grasses Points Drawing
	m_GrassShader->Bind();

	Matrix4 viewMatrix = SceneRenderer::Get()->GetCamera()->GetViewMatrix();
	Matrix4 projMatrix = SceneRenderer::Get()->GetCamera()->GetProjMatrix();

	m_GrassShader->SetMat4("viewMatrix", viewMatrix);
	m_GrassShader->SetMat4("projMatrix", projMatrix);

	//Directional Light
	if (m_DirLight == nullptr)
		m_DirLight = SceneRenderer::Get()->GetDirLight();

	m_GrassShader->SetVector3("lightDir", m_DirLight->GetLightDir());
	m_GrassShader->SetVector4("lightDirColour", m_DirLight->GetColour());
	m_GrassShader->SetFloat("lightDirIntensity", m_DirLight->GetIntensity());

	//Point Lights
	if ((int)m_PointLightsList.size() <= 0)
		m_PointLightsList = SceneRenderer::Get()->GetPointLightsList();

	m_GrassShader->SetInt("numPointLights", (int)m_PointLightsList.size());
	if ((int)m_PointLightsList.size() > 0)
	{
		for (size_t i = 0; i < m_PointLightsList.size(); i++)
		{
			LightPointNode& pointLight = *m_PointLightsList[i];

			std::string lightPosName = "pointLightPos[" + std::to_string(i) + "]";
			std::string lightColorName = "pointLightColour[" + std::to_string(i) + "]";
			std::string lightRadiusName = "pointLightRadius[" + std::to_string(i) + "]";
			std::string lightIntensityName = "pointLightIntensity[" + std::to_string(i) + "]";

			m_GrassShader->SetVector3(lightPosName, pointLight.GetLightPosition());
			m_GrassShader->SetVector4(lightColorName, pointLight.GetLightColour());
			m_GrassShader->SetFloat(lightRadiusName, pointLight.GetLightRadius());
			m_GrassShader->SetFloat(lightIntensityName, pointLight.GetLightIntensity());
		}
	}

	m_GrassShader->SetTexture("diffuseTex", m_GrassTexID, 0);
	//m_GrassShader->SetTexture("diffuseSplatmapTex", m_TerrainHMap->GetTerrainTextureSplatmap(), 1);

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	m_GrassPointMesh->DrawInstanced((int)m_GrassModelMats.size());

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_GrassShader->UnBind();

	//Draw Normals Shader
	/*m_NormalsShader->Bind();
	m_NormalsShader->SetMat4("modelMatrix", localTransform);
	m_NormalsShader->SetMat4("viewMatrix", viewMatrix);
	m_NormalsShader->SetMat4("projMatrix", projMatrix);

	m_TerrainHMap->Draw();
	m_NormalsShader->UnBind();*/
}
