#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class Mesh;
class Shader;

class AssetManager
{
public:
	AssetManager();
	~AssetManager();

	static AssetManager* Get() 
	{
		if (m_Instance == nullptr)
			m_Instance = new AssetManager();

		return m_Instance; 
	}

	std::shared_ptr<Mesh> GetMesh(const std::string& meshName, const std::string& fileName = "");
	std::shared_ptr<Shader> GetShader(const std::string& shaderName, const std::string& vertexFilename = "", const std::string& fragmentFilename = "");
	unsigned int GetTexture(const std::string& textureName, const std::string& fileName = "");

private:
	std::shared_ptr<Mesh> LoadMesh(const std::string& meshName, const std::string& fileName);
	std::shared_ptr<Shader> LoadShader(const std::string& shaderName, const std::string& vertexFilename = "", const std::string& fragmentFilename = "");
	unsigned int LoadTexture(const std::string& textureName, const std::string& fileName);

protected:
	std::unordered_map<std::string, std::shared_ptr<Mesh>> m_MeshesMap;
	std::unordered_map<std::string, std::shared_ptr<Shader>> m_ShadersMap;
	std::unordered_map<std::string, unsigned int> m_TexturesMap;

	static AssetManager* m_Instance;
};