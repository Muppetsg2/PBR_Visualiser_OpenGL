#include <Camera.h>

GLuint Camera::_uboMatrices = 0;

#if WINDOW_APP
GLuint Camera::_fbo = 0;
GLuint Camera::_rbo = 0;
GLuint Camera::_renderTexture = 0;

GLuint Camera::_vao = 0;

Shader* Camera::_renderShader = nullptr;
#endif

bool Camera::_init = false;
bool Camera::_recalculate = false;

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
glm::ivec2 Camera::_windowSize = glm::ivec2();

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

#if WINDOW_APP
bool Camera::InitFramebuffer()
{
	glGenFramebuffers(1, &Camera::_fbo);
	glGenRenderbuffers(1, &Camera::_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, Camera::_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, Camera::_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Camera::_windowSize.x, Camera::_windowSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Camera::_rbo);

	glGenTextures(1, &Camera::_renderTexture);
	glBindTexture(GL_TEXTURE_2D, Camera::_renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Camera::_windowSize.x, Camera::_windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, Camera::_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Camera::_renderTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		spdlog::error("An error occurred while creating the framebuffer");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteTextures(1, &Camera::_renderTexture);
		glDeleteRenderbuffers(1, &Camera::_rbo);
		glDeleteFramebuffers(1, &Camera::_fbo);

		Camera::_renderTexture = 0;
		Camera::_rbo = 0;
		Camera::_fbo = 0;

		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &Camera::_vao);

	return true;
}
#endif

void Camera::Init(glm::ivec2 window_size)
{
	if (Camera::_init) {
		spdlog::info("Camera already initialized!");
		return;
	}

	Camera::_windowSize = window_size;
	Camera::_recalculate = true;

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

#if WINDOW_APP
	bool res = Camera::InitFramebuffer();

	if (!res) {
		spdlog::error("The camera could not be initialized!");
		Camera::_recalculate = false;
		glDeleteBuffers(1, &Camera::_uboMatrices);
		Camera::_uboMatrices = 0;
		Camera::_windowSize = glm::ivec2();
		Camera::_window = nullptr;
		return;
	}

	_renderShader = Shader::FromExtractor("screen.vert", "screen.frag");
#endif

	Camera::_recalculate = false;
	Camera::_init = true;
}

void Camera::Init(GLFWwindow* window)
{
	if (Camera::_init) {
		spdlog::info("Camera already initialized!");
		return;
	}

	if (window == nullptr) {
		spdlog::error("Failed to initialize Camera. Window was nullptr!");
		return;
	}

	Camera::_window = window;

	glm::ivec2 s{};
	glfwGetWindowSize(Camera::_window, &s.x, &s.y);

	Camera::Init(s);
}

void Camera::Deinit()
{
	glDeleteBuffers(1, &Camera::_uboMatrices);
	Camera::_uboMatrices = 0;
	Camera::_windowSize = glm::ivec2();
	Camera::_window = nullptr;

#if WINDOW_APP
	glDeleteTextures(1, &Camera::_renderTexture);
	glDeleteRenderbuffers(1, &Camera::_rbo);
	glDeleteFramebuffers(1, &Camera::_fbo);

	Camera::_renderTexture = 0;
	Camera::_rbo = 0;
	Camera::_fbo = 0;

	glDeleteVertexArrays(1, &Camera::_vao);
	Camera::_vao = 0;

	delete _renderShader;
	_renderShader = nullptr;
#endif
}

#if WINDOW_APP
void Camera::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.f, 0.f, 0.f, 1.f);

	Camera::_renderShader->Use();
	glActiveTexture(GL_TEXTURE0 + 10);
	glBindTexture(GL_TEXTURE_2D, Camera::_renderTexture);
	Camera::_renderShader->SetInt("screenTexture", 10);

	glBindVertexArray(Camera::_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void Camera::SaveScreenshot(std::string path)
{
	// Save Image
	int width, height;
	glBindTexture(GL_TEXTURE_2D, Camera::_renderTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	unsigned char* data = new unsigned char[width * height * 3]; // GL_RGB
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
	std::string date = ss.str();

	stbi_flip_vertically_on_write(true);
	std::string name = std::format("Screenshot{}.png", date);
	int result = stbi_write_png(std::filesystem::path(std::string(path).append("\\").append(name)).string().c_str(), width, height, 3, data, 0);

	if (result != 0) {
		spdlog::info("Screenshot '{}' saved in directory '{}'", name, path);
	}
	else {
		spdlog::error("There was an error while trying to save '{}' in directory '{}'", name, path);
	}

	delete[] data;

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Camera::StartCapturing()
{
	glBindFramebuffer(GL_FRAMEBUFFER, Camera::_fbo);
	glViewport(0, 0, Camera::_windowSize.x, Camera::_windowSize.y);
}

void Camera::StopCapturing()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
#endif

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

#if WINDOW_APP
		glBindFramebuffer(GL_FRAMEBUFFER, Camera::_fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, Camera::_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Camera::_windowSize.x, Camera::_windowSize.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Camera::_rbo);

		glBindTexture(GL_TEXTURE_2D, Camera::_renderTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Camera::_windowSize.x, Camera::_windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, Camera::_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Camera::_renderTexture, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
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
	if (!Camera::_init && !Camera::_recalculate) return glm::mat4(1.f);

	glm::ivec2 size{};

	if (Camera::_window != nullptr) {
		glfwGetWindowSize(Camera::_window, &size.x, &size.y);
		Camera::_windowSize = size;
	}
	else size = Camera::_windowSize;

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
