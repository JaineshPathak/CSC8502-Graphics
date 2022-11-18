#pragma once

#include "../nclgl/common.h"
#include "../nclgl/Vector3.h"
#include "FileHandler.h"
#include <vector>

class Camera;

class CameraPathsManager
{
public:
	CameraPathsManager(const int& mode, const std::string& pathFile, Camera* cameraMain);

	void AddPathData(const Vector3& pos, const Vector3& rot);
	void Save();

	int GetMode() const { return mode; }
	void SetMode(int m) { mode = m; }

	int GetPathsDataSize() const { return (int)pathsData.size(); }

	Vector3 GetPathPos(int i) const { return pathsData[i].pathPosition; }
	Vector3 GetPathRot(int i) const { return pathsData[i].pathRotation; }

	float GetPathDelay(int i) const { return pathsData[i].delay; }
	void SetPathDelay(int i, float d) { pathsData[i].delay = d; }

	float GetSpeed() const { return speed; }
	void SetSpeed(float s) { speed = s; }

	void Update(float dt, float totalTime);

protected:
	int mode = 0;	//0 - Edit Mode, 1 - Motion Mode
	int currentPathIndex = 0;
	float currentDelayAmount = 0.0f;
	float speed = 1.0f;

	struct PathData
	{
		Vector3 pathPosition;
		Vector3 pathRotation;
		float delay = 1.0f;
	};

	std::vector<PathData> pathsData;
	Camera* cameraMain;
};