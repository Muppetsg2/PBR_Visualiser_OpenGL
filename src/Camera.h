#pragma once
#include <glm/fwd.hpp>

class Camera {
private:
	static GLuint _uboMatrices;

	static bool _init;
	static bool _recalculate;

	static float _near;
	static float _far;
	static float _fov;

	static glm::vec3 _front;
	static glm::vec3 _right;
	static glm::vec3 _up;
	static glm::vec3 _worldUp;

	static glm::vec3 _position;
	static glm::vec3 _rotation;

	static void SetFrontDir(glm::vec3 dir);
	static void OnTransformChange();

	static GLFWwindow* _window;
	static glm::ivec2 _windowSize;

public:
	static void Init(glm::ivec2 window_size);
	static void Init(GLFWwindow* window);

	static void UpdateFrontDir();
	static void OnWindowSizeChange();

	static glm::vec3 GetPosition();
	static glm::vec3 GetRotation();
	static float GetFOV();
	static float GetNearPlane();
	static float GetFarPlane();
	static glm::vec3 GetFrontDir();
	static glm::vec3 GetWorldUp();
	static glm::vec3 GetRight();
	static glm::mat4 GetViewMatrix();
	static glm::mat4 GetProjectionMatrix();

	static bool IsInitialized();

	static void SetPosition(glm::vec3 pos);
	static void SetRotation(glm::vec3 rot);
	static void SetFOV(float angle);
	static void SetFarPlane(float value);
	static void SetNearPlane(float value);
	static void SetWorldUp(glm::vec3 value);
};