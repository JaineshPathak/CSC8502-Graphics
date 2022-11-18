#include "CameraPathsManager.h"
#include "../nclgl/Camera.h"

CameraPathsManager::CameraPathsManager(const int& mode, const std::string& pathFile, Camera* cameraMain)
{
	this->mode = mode;
	this->cameraMain = cameraMain;
	if (FileHandler::FileExists(CAMERAPATHS))
	{
		std::vector<Vector3> posList, rotList;
		std::vector<float> delayList;
		FileHandler::ReadCameraPathFile(CAMERAPATHS, posList, rotList, delayList);

		if (posList.size() > 0 && rotList.size() > 0 && posList.size() == rotList.size())
		{
			for (size_t i = 0; i < posList.size(); i++)
				AddPathData(posList[i], rotList[i]);

			if (mode == 1)
			{
				cameraMain->SetPosition(posList[0]);
				cameraMain->SetRotation(rotList[0]);
			}
		}
	}
}

void CameraPathsManager::AddPathData(const Vector3& pos, const Vector3& rot)
{
	PathData path = PathData();
	path.pathPosition = pos;
	path.pathRotation = rot;
	path.delay = 1.0f;

	pathsData.push_back(path);
}

void CameraPathsManager::Save()
{
	if (pathsData.size() == 0)
		return;

	std::vector<Vector3> camPos, camRot;
	std::vector<float> delayList;
	for (size_t i = 0; i < pathsData.size(); i++)
	{
		camPos.push_back(pathsData[i].pathPosition);
		camRot.push_back(pathsData[i].pathRotation);
		delayList.push_back(pathsData[i].delay);
	}

	FileHandler::SaveCameraPathFile(CAMERAPATHS, camPos, camRot, delayList);
}

void CameraPathsManager::Update(float dt, float totalTime)
{
	if (mode != 1)
		return;
	else if (mode == 1 && pathsData.size() <= 0)
		return;

	Vector3 currentPointPos = pathsData[currentPathIndex].pathPosition;
	Vector3 currentPointRot = pathsData[currentPathIndex].pathRotation;

	Vector3 currentCamPos = cameraMain->getPosition();
	currentCamPos = Vector3::Lerp(currentCamPos, currentPointPos, speed * dt);
	cameraMain->SetPosition(currentCamPos);

	Vector3 currentCamRot = cameraMain->getRotation();
	currentCamRot = Vector3::Lerp(currentCamRot, currentPointRot, speed * dt);
	cameraMain->SetRotation(currentCamRot);
	//cameraMain->SetPosition(Vector3::Lerp(cameraMain->getPosition(), currentPointPos, 2.0f * dt));
	//cameraMain->SetRotation(Vector3::Lerp(cameraMain->getRotation(), currentPointRot, 2.0f * dt));

	float distance = Vector3::Distance(cameraMain->getPosition(), currentPointPos);
	if (distance <= 1.0f)
	{
		currentDelayAmount += dt;
		if (currentDelayAmount >= pathsData[currentPathIndex].delay)
		{
			currentPathIndex++;
			currentDelayAmount = 0.0f;
		}
	}

	if (currentPathIndex >= pathsData.size())
		currentPathIndex = 0;
}