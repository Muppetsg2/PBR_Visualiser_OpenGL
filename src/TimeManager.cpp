#include <TimeManager.h>

float TimeManager::_updateDeltaTime = 0.f;
float TimeManager::_timeMultiplier = 1.0f;

void TimeManager::Update()
{
	static float t1 = glfwGetTime();
	static float t2 = glfwGetTime();

	t1 = t2;
	t2 = glfwGetTime();
	_updateDeltaTime = (t2 - t1) * _timeMultiplier;
}

float TimeManager::GetTime()
{
	return glfwGetTime();
}

float TimeManager::GetDeltaTime()
{
	return _updateDeltaTime;
}