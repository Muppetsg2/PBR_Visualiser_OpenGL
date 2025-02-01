#pragma once

class TimeManager {
private:
	static float _updateDeltaTime;

public:
	static float timeMultiplier;
	static void Update();
	static float GetTime();
	static float GetDeltaTime();
};