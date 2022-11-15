#include "FileHandler.h"

FileHandler::FileHandler()
{
}

FileHandler::FileHandler(std::vector<std::string>& outputSetStr)
{
	readOutputSetFile(outputSetStr);
}

FileHandler::~FileHandler()
{
}

void FileHandler::readOutputSetFile(std::vector<std::string>& outputSetStr)
{
	std::ifstream fileReader(m_outputSetFilename, std::ios::in);
	fileReader.exceptions(std::ifstream::badbit);
	try
	{
		if (!fileReader.good())	throw std::ifstream::failure("Probably file doesn't exists or Badbit Error");
		if (fileReader.peek() == EOF) throw std::ifstream::failure("File is Empty!");

		std::string line/*, finalLine*/;
		while (std::getline(fileReader, line, '\n'))
		{
			line.erase(remove(line.begin(), line.end(), ','), line.end());
			outputSetStr.push_back(line);
		}
		fileReader.close();
	}
	catch (std::ifstream::failure& e) {
		std::cout << "\nException Occured: Failed reading file: " << m_outputSetFilename << "\nMessage: " << e.what();
	}
}

void FileHandler::saveFile(const std::vector<int>& outputSet)
{
	std::ofstream fileWriter(m_outputSetFilename, std::ios::app);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	try
	{
		for (size_t i = 0; i < outputSet.size(); i++)
			(i == outputSet.size() - 1) ? fileWriter << outputSet[i] : fileWriter << outputSet[i] << ", ";
		fileWriter << "\n";
		fileWriter.close();
	}
	catch (std::ofstream::failure& e) {
		std::cout << "\nException Occured: Failed writing to file: " << m_outputSetFilename << "\nMessage: " << e.what();
	}
	std::cout << "\nFile Saved: " << m_outputSetFilename;
}

void FileHandler::saveFile(const std::string& expressionStr)
{
	std::ofstream fileWriter(m_expressionFilename, std::ios::out);
	fileWriter.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	try
	{
		fileWriter << expressionStr;
		fileWriter.close();
	}
	catch (std::ofstream::failure& e)
	{
		std::cout << "\nException Occured: Failed writing to file: " << m_expressionFilename << "\nMessage: " << e.what();
	}
	std::cout << "\nFile Saved: " << m_expressionFilename;
}

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
	
	int PropPosSize = PropPos.size();
	int PropRotSize = PropRot.size();
	int PropScaleSize = PropScale.size();

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