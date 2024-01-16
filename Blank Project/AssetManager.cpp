#include "AssetManager.h"

#include <nclgl/Mesh.h>
#include <nclgl/Shader.h>

#include <SOIL/SOIL.h>

AssetManager* AssetManager::m_Instance = nullptr;

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& meshName, const std::string& fileName)
{
	auto iterator = m_MeshesMap.find(meshName);
	if (iterator != m_MeshesMap.end())
		return iterator->second;

	return LoadMesh(meshName, fileName);
}

std::shared_ptr<Shader> AssetManager::GetShader(const std::string& shaderName, const std::string& vertexFilename, const std::string& fragmentFilename)
{
	auto iterator = m_ShadersMap.find(shaderName);
	if (iterator != m_ShadersMap.end())
		return iterator->second;

	return LoadShader(shaderName, vertexFilename, fragmentFilename);
}

unsigned int AssetManager::GetTexture(const std::string& textureName, const std::string& fileName)
{
	auto iterator = m_TexturesMap.find(textureName);
	if (iterator != m_TexturesMap.end())
		return iterator->second;

	return LoadTexture(textureName, fileName);
}

std::shared_ptr<Mesh> AssetManager::LoadMesh(const std::string& meshName, const std::string& fileName)
{
	std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(Mesh::LoadFromMeshFile(fileName));
	m_MeshesMap[meshName] = mesh;
	
	return mesh;
}

std::shared_ptr<Shader> AssetManager::LoadShader(const std::string& shaderName, const std::string& vertexFilename, const std::string& fragmentFilename)
{
	std::shared_ptr<Shader> shader = std::shared_ptr<Shader>(new Shader(vertexFilename, fragmentFilename));
	m_ShadersMap[shaderName] = shader;
	
	return shader;
}

unsigned int AssetManager::LoadTexture(const std::string& textureName, const std::string& fileName)
{
	unsigned int texID = SOIL_load_OGL_texture(fileName.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	m_TexturesMap[textureName] = texID;

	return texID;
}