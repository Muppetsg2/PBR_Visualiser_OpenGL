#include <Camera.h>

GLuint Camera::_uboMatrices = 0;

bool Camera::_init = false;

float Camera::_near = 0.1f;
float Camera::_far = 1000.f;
float Camera::_fov = 45.f;

glm::vec3 Camera::_front = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 Camera::_right = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 Camera::_up = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 Camera::_worldUp = glm::vec3(0.f, 1.f, 0.f);

glm::vec3 Camera::_position = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 Camera::_rotation = glm::vec3(0.f, 0.f, 0.f);

GLFWwindow* Camera::_window = nullptr;

void Camera::SetFrontDir(glm::vec3 dir)
{
	Camera::_front = glm::normalize(dir);
	Camera::_right = glm::normalize(glm::cross(Camera::_front, Camera::_worldUp));
	Camera::_up = glm::normalize(glm::cross(Camera::_right, Camera::_front));

	if (Camera::_init) {
		glBindBuffer(GL_UNIFORM_BUFFER, Camera::_uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(Camera::GetViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void Camera::OnTransformChange()
{
	UpdateFrontDir();
}

void Camera::Init(GLFWwindow* window)
{
	if (window == nullptr) {
		spdlog::error("Failed to initialize Camera. Window was nullptr!");
		return;
	}

	Camera::_window = window;

	UpdateFrontDir();

	glGenBuffers(1, &Camera::_uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, Camera::_uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, Camera::_uboMatrices, 0, 2 * sizeof(glm::mat4));

	glBindBuffer(GL_UNIFORM_BUFFER, Camera::_uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(Camera::GetProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(Camera::GetViewMatrix()));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Camera::_init = true;
}

void Camera::UpdateFrontDir()
{
	glm::vec3 rot = Camera::_rotation;

	glm::vec3 front{};
	front.x = cos(glm::radians(rot.y)) * cos(glm::radians(rot.x));
	front.y = sin(glm::radians(rot.x));
	front.z = sin(glm::radians(rot.y)) * cos(glm::radians(rot.x));
	Camera::SetFrontDir(glm::normalize(front));
}

void Camera::OnWindowSizeChange()
{
	if (Camera::_init) {
		glBindBuffer(GL_UNIFORM_BUFFER, Camera::_uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(Camera::GetProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

glm::vec3 Camera::GetPosition()
{
	return Camera::_position;
}

glm::vec3 Camera::GetRotation()
{
	return Camera::_rotation;
}

float Camera::GetFOV()
{
	return Camera::_fov;
}

float Camera::GetNearPlane()
{
	return Camera::_near;
}

float Camera::GetFarPlane()
{
	return Camera::_far;
}

glm::vec3 Camera::GetFrontDir()
{
	return Camera::_front;
}

glm::vec3 Camera::GetWorldUp()
{
	return Camera::_worldUp;
}

glm::vec3 Camera::GetRight()
{
	return Camera::_right;
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Camera::_position, Camera::_position + Camera::_front, Camera::_up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
	if (!Camera::_init && Camera::_window == nullptr) return glm::mat4(1.f);

	glm::ivec2 size{};
	glfwGetWindowSize(Camera::_window, &size.x, &size.y);
	return glm::perspective(glm::radians(Camera::_fov), (size.y != 0) ? ((float)size.x / (float)size.y) : 0, Camera::_near, Camera::_far);
}

bool Camera::IsInitialized()
{
	return Camera::_init;
}

void Camera::SetPosition(glm::vec3 pos)
{
	if (Camera::_position == pos) return;

	Camera::_position = pos;
	Camera::OnTransformChange();
}

void Camera::SetRotation(glm::vec3 rot)
{
	if (Camera::_rotation == rot) return;

	Camera::_rotation = rot;
	Camera::OnTransformChange();
}

void Camera::SetFOV(float angle)
{
	Camera::_fov = angle;
}

void Camera::SetFarPlane(float value)
{
	Camera::_far = value;
}

void Camera::SetNearPlane(float value)
{
	Camera::_near = value;
}

void Camera::SetWorldUp(glm::vec3 value)
{
	Camera::_worldUp = glm::normalize(value);
	Camera::_right = glm::normalize(glm::cross(Camera::_front, Camera::_worldUp));
	Camera::_up = glm::normalize(glm::cross(Camera::_right, Camera::_front));
}
