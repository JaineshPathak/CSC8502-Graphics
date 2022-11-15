#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include "../nclgl/Vector3.h"

class FileHandler
{
private:
	const std::string m_outputSetFilename = "OutputSet.txt";
	const std::string m_expressionFilename = "Expression.txt";

public:
	FileHandler();
	FileHandler(std::vector<std::string>& outputSetStr);
	~FileHandler();

	void readOutputSetFile(std::vector<std::string>& outputSetStr);
	void saveFile(const std::vector<int>& outputSet);
	void saveFile(const std::string& expressionStr);

	static bool FileExists(const std::string& fileName);
	static void SavePropDataToFile(const std::string& fileName, const std::vector<Vector3>& PropPos, const std::vector<Vector3>& PropRot, const std::vector<Vector3>& PropScale);
	static void ReadPropDataFromFile(const std::string& fileName, std::vector<Vector3>& PropPos, std::vector<Vector3>& PropRot, std::vector<Vector3>& PropScale);
};
