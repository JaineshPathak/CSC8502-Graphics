#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/Light.h"
#include "../nclgl/DirectionalLight.h"

class LightPointNode;
class FileHandler
{
public:
	FileHandler() {};
	~FileHandler() {};

	static bool FileExists(const std::string& fileName);
	static void SavePropDataToFile(const std::string& fileName, const std::vector<Vector3>& PropPos, const std::vector<Vector3>& PropRot, const std::vector<Vector3>& PropScale);
	static void ReadPropDataFromFile(const std::string& fileName, std::vector<Vector3>& PropPos, std::vector<Vector3>& PropRot, std::vector<Vector3>& PropScale);

	static void SaveFogFile(const std::string& fileName, const bool& fogEnabled, const Vector4& fogColour);
	static void ReadFogFile(const std::string& fileName, bool& fogEnabled, Vector4& fogColour);

	static void SaveLightDataFile(const std::string& fileName, const DirectionalLight& dirLight, const std::vector<Light>& pointLightsData);
	static void ReadLightDataFile(const std::string& fileName, DirectionalLight& dirLight, std::vector<Light>& pointLightsData);
	static void ReadLightDataFile(const std::string& fileName, DirectionalLight& dirLight, std::vector<std::shared_ptr<LightPointNode>>& pointLightsData);
	static void ReadLightDataFile(const std::string& fileName, std::vector<float>& pLightIntensityV, std::vector<float>& pLightRadiusV, std::vector<Vector3>& pLightPosV, std::vector<Vector4>& pLightColorV, std::vector<Vector4>& pLightSpecColorV);

	static void SaveCameraPathFile(const std::string& fileName, const std::vector<Vector3>& camPosV, const std::vector<Vector3>& camRotV, const std::vector<float>& camDelayV);
	static void ReadCameraPathFile(const std::string& fileName, std::vector<Vector3>& camPosV, std::vector<Vector3>& camRotV, std::vector<float>& camDelayV);
};
