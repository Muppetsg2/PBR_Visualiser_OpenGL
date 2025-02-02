#pragma once

#if WINDOW_APP
#include <Shader.h>
#endif

class Camera {
private:
	static GLuint _uboMatrices;

#if WINDOW_APP
	static GLuint _fbo;
	static GLuint _rbo;
	static GLuint _renderTexture;

	static GLuint _vao;

	static Shader* _renderShader;
#endif

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

	static GLFWwindow* _window;
	static glm::ivec2 _windowSize;

	static void SetFrontDir(glm::vec3 dir);
	static void OnTransformChange();

#if WINDOW_APP
	static bool InitFramebuffer();
#endif

	Camera() = delete;

public:
	static void Init(glm::ivec2 window_size);
	static void Init(GLFWwindow* window);
	static void Deinit();

#if WINDOW_APP
	static void Render();
	static void SaveScreenshot(std::string path);

	static void StartCapturing();
	static void StopCapturing();
#endif

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