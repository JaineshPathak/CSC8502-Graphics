#include "FileHandler.h"
#include "LightPointNode.h"

//----------------------------------------------------------------------------------------------------------

bool FileHandler::FileExists(const std::string& fileName)
{
	std::ifstream f(fileName.c_str());
	return f.good();
}

void FileHandler::SavePropDataToFile(const std::string& fileName, const std::vector<Vector3>& PropPos, const std::vector<Vector3>& PropRot, const std::vector<Vector3>& PropScale)
{
	std::ofstream fileWriter(fileName, std::ios::out);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	
	int PropPosSize = (int)PropPos.size();
	int PropRotSize = (int)PropRot.size();
	int PropScaleSize = (int)PropScale.size();

	fileWriter << PropPosSize << std::endl;
	for (int i = 0; i < PropPosSize; i++)
		fileWriter << PropPos[i].x << " " << PropPos[i].y << " " << PropPos[i].z << "\n";

	fileWriter << "EndPos\n";

	fileWriter << PropRotSize << std::endl;
	for (int i = 0; i < PropPosSize; i++)
		fileWriter << PropRot[i].x << " " << PropRot[i].y << " " << PropRot[i].z << "\n";

	fileWriter << "EndRot\n";

	fileWriter << PropScaleSize << std::endl;
	for (int i = 0; i < PropScaleSize; i++)
		fileWriter << PropScale[i].x << " " << PropScale[i].y << " " << PropScale[i].z << "\n";

	fileWriter << "EndScale\n";

	fileWriter.close();
	std::cout << "\nFile Saved: " << fileName;
}

void FileHandler::ReadPropDataFromFile(const std::string& fileName, std::vector<Vector3>& PropPos, std::vector<Vector3>& PropRot, std::vector<Vector3>& PropScale)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	int PropPosSize, PropRotSize, PropScaleSize;
	Vector3 pos, rot, scale;
	std::string line;

	//Position
	fileReader >> PropPosSize;
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPos")
			break;
		
		std::stringstream ss(line);
		if (line != "")
		{
			ss >> (float)pos.x;
			ss >> (float)pos.y;
			ss >> (float)pos.z;
			PropPos.push_back(pos);
		}
	}
	
	//Rotation
	fileReader >> PropRotSize;
	//PropRot.resize(PropRotSize);
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndRot")
			break;
		
		std::stringstream ss(line);
		if (line != "")
		{
			ss >> (float)rot.x;
			ss >> (float)rot.y;
			ss >> (float)rot.z;
			PropRot.push_back(rot);
			//std::cout << "Rotation = " << rot << std::endl;
		}
	}

	//Scale
	fileReader >> PropScaleSize;
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndScale")
			break;
		
		std::stringstream ss(line);
		if (line != "")
		{
			ss >> (float)scale.x;
			ss >> (float)scale.y;
			ss >> (float)scale.z;
			PropScale.push_back(scale);
			//std::cout << "Scale = " << scale << std::endl;
		}
	}
	fileReader.close();
}



void FileHandler::SaveFogFile(const std::string& fileName, const bool& fogEnabled, const Vector4& fogColour)
{
	std::ofstream fileWriter(fileName, std::ios::out);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	fileWriter << fogEnabled << std::endl;
	fileWriter << fogColour.x << " " << fogColour.y << " " << fogColour.z << " " << fogColour.w << "\n";

	fileWriter.close();
	std::cout << "\nFile Saved: " << fileName;
}

void FileHandler::ReadFogFile(const std::string& fileName, bool& fogEnabled, Vector4& fogColour)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	std::string line;

	fileReader >> fogEnabled;
	while (std::getline(fileReader, line, '\n'))
	{
		std::stringstream ss(line);
		if (line != "")
		{
			ss >> (float)fogColour.x;
			ss >> (float)fogColour.y;
			ss >> (float)fogColour.z;
			ss >> (float)fogColour.w;
			//std::cout << "Scale = " << scale << std::endl;
		}
	}
	fileReader.close();
}



void FileHandler::SaveLightDataFile(const std::string& fileName, const DirectionalLight& dirLight, const std::vector<Light>& pointLightsData)
{
	std::ofstream fileWriter(fileName, std::ios::out);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	//Direction
	fileWriter << dirLight.GetLightDir().x << " " << dirLight.GetLightDir().y << " " << dirLight.GetLightDir().z << std::endl;
	//Colour
	fileWriter << dirLight.GetColour().x << " " << dirLight.GetColour().y << " " << dirLight.GetColour().z << " " << dirLight.GetColour().w << std::endl;
	//Intensity
	fileWriter << dirLight.GetIntensity() << std::endl;

	if (pointLightsData.size() > 0)
	{
		for (size_t i = 0; i < pointLightsData.size(); i++)
			fileWriter << pointLightsData[i].GetPosition().x << " " << pointLightsData[i].GetPosition().y << " " << pointLightsData[i].GetPosition().z << std::endl;
		fileWriter << "EndPointLightPos" << std::endl;

		for (size_t i = 0; i < pointLightsData.size(); i++)
			fileWriter << pointLightsData[i].GetColour().x << " " << pointLightsData[i].GetColour().y << " " << pointLightsData[i].GetColour().z << " " << pointLightsData[i].GetColour().w << std::endl;
		fileWriter << "EndPointLightColour" << std::endl;

		for (size_t i = 0; i < pointLightsData.size(); i++)
			fileWriter << pointLightsData[i].GetSpecularColour().x << " " << pointLightsData[i].GetSpecularColour().y << " " << pointLightsData[i].GetSpecularColour().z << " " << pointLightsData[i].GetSpecularColour().w << std::endl;
		fileWriter << "EndPointLightSpecularColour" << std::endl;

		for (size_t i = 0; i < pointLightsData.size(); i++)
			fileWriter << pointLightsData[i].GetRadius() << std::endl;
		fileWriter << "EndPointLightRadius" << std::endl;

		for (size_t i = 0; i < pointLightsData.size(); i++)
			fileWriter << pointLightsData[i].GetIntensity() << std::endl;
		fileWriter << "EndPointLightIntensity" << std::endl;
	}

	fileWriter.close();
	std::cout << "\nFile Saved: " << fileName;
}

void FileHandler::ReadLightDataFile(const std::string& fileName, DirectionalLight& dirLight, std::vector<Light>& pointLightsData)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	std::string line;

	Vector3 dirLightDir;
	fileReader >> (float)dirLightDir.x;
	fileReader >> (float)dirLightDir.y;
	fileReader >> (float)dirLightDir.z;
	//dirLight.SetLightDir(dirLightDir);

	Vector4 dirLightColour;
	fileReader >> (float)dirLightColour.x;
	fileReader >> (float)dirLightColour.y;
	fileReader >> (float)dirLightColour.z;
	fileReader >> (float)dirLightColour.w;
	dirLight.SetColour(dirLightColour);

	float dirLightIntensity;
	fileReader >> (float)dirLightIntensity;
	dirLight.SetIntensity(dirLightIntensity);

	//==================================================================
	std::vector<Vector3> pLightPos;
	std::vector<Vector4> pLightColour;
	std::vector<Vector4> pLightSpecColour;
	std::vector<float> pLightRadius;
	std::vector<float> pLightIntensity;
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightPos")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector3 lightPos;
			ss >> (float)lightPos.x;
			ss >> (float)lightPos.y;
			ss >> (float)lightPos.z;
			pLightPos.push_back(lightPos);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightCol;
			ss >> (float)lightCol.x;
			ss >> (float)lightCol.y;
			ss >> (float)lightCol.z;
			ss >> (float)lightCol.w;
			pLightColour.push_back(lightCol);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightSpecularColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightColS;
			ss >> (float)lightColS.x;
			ss >> (float)lightColS.y;
			ss >> (float)lightColS.z;
			ss >> (float)lightColS.w;
			pLightSpecColour.push_back(lightColS);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightRadius")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightR;
			ss >> (float)lightR;
			pLightRadius.push_back(lightR);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightIntensity")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightI;
			ss >> (float)lightI;
			pLightIntensity.push_back(lightI);
		}
	}

	for (size_t i = 0; i < pLightPos.size(); i++)
	{
		Light pointLight;
		pointLight.SetPosition(pLightPos[i]);
		pointLight.SetColour(pLightColour[i]);
		pointLight.SetSpecularColour(pLightSpecColour[i]);
		pointLight.SetRadius(pLightRadius[i]);
		pointLight.SetIntensity(pLightIntensity[i]);
		pointLightsData.push_back(pointLight);
	}

	fileReader.close();
}

void FileHandler::ReadLightDataFile(const std::string& fileName, DirectionalLight& dirLight, std::vector<std::shared_ptr<LightPointNode>>& pointLightsData)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	std::string line;

	Vector3 dirLightDir;
	fileReader >> (float)dirLightDir.x;
	fileReader >> (float)dirLightDir.y;
	fileReader >> (float)dirLightDir.z;
	//dirLight.SetLightDir(dirLightDir);

	Vector4 dirLightColour;
	fileReader >> (float)dirLightColour.x;
	fileReader >> (float)dirLightColour.y;
	fileReader >> (float)dirLightColour.z;
	fileReader >> (float)dirLightColour.w;
	dirLight.SetColour(dirLightColour);

	float dirLightIntensity;
	fileReader >> (float)dirLightIntensity;
	dirLight.SetIntensity(dirLightIntensity);

	//==================================================================
	std::vector<Vector3> pLightPos;
	std::vector<Vector4> pLightColour;
	std::vector<Vector4> pLightSpecColour;
	std::vector<float> pLightRadius;
	std::vector<float> pLightIntensity;
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightPos")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector3 lightPos;
			ss >> (float)lightPos.x;
			ss >> (float)lightPos.y;
			ss >> (float)lightPos.z;
			pLightPos.push_back(lightPos);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightCol;
			ss >> (float)lightCol.x;
			ss >> (float)lightCol.y;
			ss >> (float)lightCol.z;
			ss >> (float)lightCol.w;
			pLightColour.push_back(lightCol);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightSpecularColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightColS;
			ss >> (float)lightColS.x;
			ss >> (float)lightColS.y;
			ss >> (float)lightColS.z;
			ss >> (float)lightColS.w;
			pLightSpecColour.push_back(lightColS);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightRadius")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightR;
			ss >> (float)lightR;
			pLightRadius.push_back(lightR);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightIntensity")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightI;
			ss >> (float)lightI;
			pLightIntensity.push_back(lightI);
		}
	}

	for (size_t i = 0; i < pLightPos.size(); i++)
	{
		std::shared_ptr<LightPointNode> pointLight = std::shared_ptr<LightPointNode>(new LightPointNode());
		pointLight->SetLightColour(pLightColour[i]);
		pointLight->SetLightSpecularColour(pLightSpecColour[i]);
		pointLight->SetLightRadius(pLightRadius[i]);
		pointLight->SetLightIntensity(pLightIntensity[i]);
		pointLight->SetPosition(pLightPos[i]);
		pointLightsData.push_back(pointLight);
	}

	fileReader.close();
}

void FileHandler::ReadLightDataFile(const std::string& fileName, std::vector<float>& pLightIntensityV, std::vector<float>& pLightRadiusV, std::vector<Vector3>& pLightPosV, std::vector<Vector4>& pLightColorV, std::vector<Vector4>& pLightSpecColorV)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	std::string line;

	Vector3 dirLightDir;
	fileReader >> (float)dirLightDir.x;
	fileReader >> (float)dirLightDir.y;
	fileReader >> (float)dirLightDir.z;
	//dirLight.SetLightDir(dirLightDir);

	Vector4 dirLightColour;
	fileReader >> (float)dirLightColour.x;
	fileReader >> (float)dirLightColour.y;
	fileReader >> (float)dirLightColour.z;
	fileReader >> (float)dirLightColour.w;

	float dirLightIntensity;
	fileReader >> (float)dirLightIntensity;

	//==================================================================
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightPos")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector3 lightPos;
			ss >> (float)lightPos.x;
			ss >> (float)lightPos.y;
			ss >> (float)lightPos.z;
			pLightPosV.emplace_back(lightPos);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightCol;
			ss >> (float)lightCol.x;
			ss >> (float)lightCol.y;
			ss >> (float)lightCol.z;
			ss >> (float)lightCol.w;
			pLightColorV.emplace_back(lightCol);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightSpecularColour")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector4 lightColS;
			ss >> (float)lightColS.x;
			ss >> (float)lightColS.y;
			ss >> (float)lightColS.z;
			ss >> (float)lightColS.w;
			pLightSpecColorV.emplace_back(lightColS);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightRadius")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightR;
			ss >> (float)lightR;
			pLightRadiusV.emplace_back(lightR);
		}
	}


	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPointLightIntensity")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float lightI;
			ss >> (float)lightI;
			pLightIntensityV.emplace_back(lightI);
		}
	}

	fileReader.close();
}

void FileHandler::SaveCameraPathFile(const std::string& fileName, const std::vector<Vector3>& camPosV, const std::vector<Vector3>& camRotV, const std::vector<float>& camDelayV)
{
	std::ofstream fileWriter(fileName, std::ios::out);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	for (size_t i = 0; i < camPosV.size(); i++)
		fileWriter << camPosV[i].x << " " << camPosV[i].y << " " << camPosV[i].z << "\n";

	fileWriter << "EndPos\n";

	for (size_t i = 0; i < camRotV.size(); i++)
		fileWriter << camRotV[i].x << " " << camRotV[i].y << " " << camRotV[i].z << "\n";

	fileWriter << "EndRot\n";

	for (size_t i = 0; i < camDelayV.size(); i++)
		fileWriter << camDelayV[i] << "\n";

	fileWriter << "EndCamDelay\n";

	fileWriter.close();
	std::cout << "\nFile Saved: " << fileName;
}

void FileHandler::ReadCameraPathFile(const std::string& fileName, std::vector<Vector3>& camPosV, std::vector<Vector3>& camRotV, std::vector<float>& camDelayV)
{
	std::ifstream fileReader(fileName, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);

	std::string line;

	//Position
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndPos")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector3 pos;
			ss >> (float)pos.x;
			ss >> (float)pos.y;
			ss >> (float)pos.z;
			camPosV.push_back(pos);
		}
	}

	//Rotation
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndRot")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			Vector3 rot;
			ss >> (float)rot.x;
			ss >> (float)rot.y;
			ss >> (float)rot.z;
			camRotV.push_back(rot);
			//std::cout << "Rotation = " << rot << std::endl;
		}
	}

	//Delay
	while (std::getline(fileReader, line, '\n'))
	{
		if (line == "EndCamDelay")
			break;

		std::stringstream ss(line);
		if (line != "")
		{
			float delay;
			ss >> (float)delay;
			camDelayV.push_back(delay);
			//std::cout << "Rotation = " << rot << std::endl;
		}
	}
	fileReader.close();
}