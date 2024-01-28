#include "TerrainHeightmap.h"
#include <iostream>

#include <nclgl/Shader.h>
#include "AssetManager.h"

TerrainHeightmap::TerrainHeightmap(const std::string& name, float vertexScaleWidth, float vertexScaleLength, float texturingScale, float extraHeight):
	m_TerrainTextureSplatmap(-1),
	m_TerrainTextureGrass(-1),
	m_TerrainTextureGrassBump(-1),
	m_TerrainTextureRocks(-1),
	m_TerrainTextureRocksBump(-1),
	m_TerrainTextureGround(-1),
	m_TerrainTextureGroundBump(-1)
{
	int iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(name.c_str(), &iWidth, &iHeight, &iChans, 1);

	if (!data)
	{
		m_InitSuccess = false;
		std::cout << "Terrain Heightmap - can't load image file!\n";
		return;
	}

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	colours = new Vector4[numVertices];

	Vector3 vertexScaleV = Vector3(vertexScaleWidth, 1.0f, vertexScaleLength);
	Vector2 textureScale = Vector2(1 / texturingScale, 1 / texturingScale);

	for (int z = 0; z < iHeight; ++z)
	{
		for (int x = 0; x < iWidth; ++x)
		{
			int offset = (z * iWidth) + x;
			vertices[offset] = Vector3(x, (float)data[offset] * extraHeight, z) * vertexScaleV;
			textureCoords[offset] = Vector2(x, z) * textureScale;

			float color = (float)data[offset] / iWidth;
			colours[offset] = Vector4(color, color, color, 1.0f);
		}
	}
	SOIL_free_image_data(data);

	int i = 0;
	for (int z = 0; z < iHeight - 1; ++z)
	{
		for (int x = 0; x < iWidth - 1; ++x)
		{
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = ((z + 1) * (iWidth)) + (x + 1);
			int d = ((z + 1) * (iWidth)) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}
	GenerateNormals();
	//FlipNormals();
	GenerateTangents();
	BufferData();

	heightMapSize.x = vertexScaleV.x * (iWidth - 1);
	heightMapSize.y = vertexScaleV.y * 255.0f;
	heightMapSize.z = vertexScaleV.z * (iHeight - 1);
	
	//--------------------------------------------------------------------------
	// Shader loading
	//terrainShader = new Shader(SHADERDIRCOURSETERRAIN"CWTexturedVertexv2.glsl", SHADERDIRCOURSETERRAIN"TerrainFragv2.glsl");
	m_TerrainShader = AssetManager::Get()->GetShader("TerrainShader", SHADERDIRCOURSETERRAIN"CWTexturedVertexv2.glsl", SHADERDIRCOURSETERRAIN"CWTerrainFragv2.glsl");
	if (!m_TerrainShader->LoadSuccess())
	{
		m_InitSuccess = false;
		std::cout << "Terrain Heightmap - Something went wrong with Shader!\n";
		return;
	}
	//--------------------------------------------------------------------------
	// Textures
	
	/*terrainTextureSplatmap = SOIL_load_OGL_texture(TEXTUREDIRCOURSETERRAIN"Terrain_Splatmap4.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTextureGrass = SOIL_load_OGL_texture(TEXTUREDIRCOURSETERRAIN"Terrain_Grass_D.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTextureRocks = SOIL_load_OGL_texture(TEXTUREDIRCOURSETERRAIN"Terrain_Rocks_D.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTextureGround = SOIL_load_OGL_texture(TEXTUREDIRCOURSETERRAIN"Terrain_Ground_D.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);*/
	m_TerrainTextureSplatmap = AssetManager::Get()->GetTexture("TerrainSplatMap", TEXTUREDIRCOURSETERRAIN"Terrain_Splatmap4.png", false);
	m_TerrainTextureGrass = AssetManager::Get()->GetTexture("TerrainGrass_Diffuse", TEXTUREDIRCOURSETERRAIN"Terrain_Grass_D.png", false);
	m_TerrainTextureGrassBump = AssetManager::Get()->GetTexture("TerrainGrass_Bump", TEXTUREDIRCOURSETERRAIN"Terrain_Grass_N.png", false);
	m_TerrainTextureRocks = AssetManager::Get()->GetTexture("TerrainRocks_Diffuse", TEXTUREDIRCOURSETERRAIN"Terrain_Rocks_D.png", false);
	m_TerrainTextureRocksBump = AssetManager::Get()->GetTexture("TerrainRocks_Bump", TEXTUREDIRCOURSETERRAIN"Terrain_Rocks_N.png", false);
	m_TerrainTextureGround = AssetManager::Get()->GetTexture("TerrainGround_Diffuse", TEXTUREDIRCOURSETERRAIN"Terrain_Ground_D.png", false);
	m_TerrainTextureGroundBump = AssetManager::Get()->GetTexture("TerrainGround_Bump", TEXTUREDIRCOURSETERRAIN"Terrain_Ground_N.png", false);
	if (!m_TerrainTextureSplatmap || 
		!m_TerrainTextureGrass ||
		!m_TerrainTextureGrassBump ||
		!m_TerrainTextureGround || 
		!m_TerrainTextureGroundBump || 
		!m_TerrainTextureRocks ||
		!m_TerrainTextureRocksBump)
	{
		m_InitSuccess = false;
		std::cout << "Terrain Heightmap - Something went wrong with Terrain Textures!\n";
		return;
	}

	OGLRenderer::SetTextureRepeating(m_TerrainTextureGrass, true);
	OGLRenderer::SetTextureRepeating(m_TerrainTextureGrassBump, true);
	OGLRenderer::SetTextureRepeating(m_TerrainTextureRocks, true);
	OGLRenderer::SetTextureRepeating(m_TerrainTextureRocksBump, true);
	OGLRenderer::SetTextureRepeating(m_TerrainTextureGround, true);
	OGLRenderer::SetTextureRepeating(m_TerrainTextureGroundBump, true);
	//--------------------------------------------------------------------------

	m_InitSuccess = true;
}

TerrainHeightmap::~TerrainHeightmap()
{
	glDeleteTextures(1, &m_TerrainTextureSplatmap);
	glDeleteTextures(1, &m_TerrainTextureGrass);
	glDeleteTextures(1, &m_TerrainTextureGrassBump);
	glDeleteTextures(1, &m_TerrainTextureRocks);
	glDeleteTextures(1, &m_TerrainTextureRocksBump);
	glDeleteTextures(1, &m_TerrainTextureGround);
	glDeleteTextures(1, &m_TerrainTextureGroundBump);
}