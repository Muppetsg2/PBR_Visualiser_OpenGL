#pragma once

class CameraController {
private:
	static bool _mouseNotUsed;

	static GLfloat _lastX;
	static GLfloat _lastY;

	static float _cameraSpeedDefault;
	static float _sensitivityDefault;

public:
	static float cameraSpeed;
	static float sensitivity;

	static void MoveCamera(GLFWwindow* window);
	static void RotateCamera(double xpos, double ypos);
	
	static void ResetRotation();

	static void DrawEditor(bool* open);
};