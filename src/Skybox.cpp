#include <Skybox.h>
#include <Shape.h>

GLuint Skybox::_vao = 0;
GLuint Skybox::_texture = 0;
Shader* Skybox::_shader = nullptr;

GLuint Skybox::_irradianceTexture = 0;

bool Skybox::_init = false;
bool Skybox::_hdr = false;

const GLchar* Skybox::_paths[6] = { "", "", "", "", "", "" };
GLFWwindow* Skybox::_window = nullptr;

void Skybox::Init(GLFWwindow* window, const GLchar* faces[6])
{
	if (Skybox::_init) {
		spdlog::info("Skybox already initialized!");
		return;
	}

	if (window == nullptr) {
		spdlog::info("Window was nullptr!");
		return;
	}

	if (Skybox::_shader == nullptr) {
		Skybox::_shader = new Shader("./res/shader/skybox.vert", "./res/shader/skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	Shader* irradianceShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	Skybox::_window = window;

	glGenVertexArrays(1, &Skybox::_vao);
	glBindVertexArray(Skybox::_vao);

	glBindBuffer(GL_ARRAY_BUFFER, Shape::GetCubeVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetCubeEBO());

	// Vertices positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Skybox::_hdr = false;

	glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	bool err = false;
	int width, height, nrChannels;
	GLenum inter = GL_SRGB;
	GLenum format = GL_RGB;
	for (size_t i = 0; i < 6; ++i)
	{
		Skybox::_paths[i] = faces[i];

		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			if (nrChannels == 1) { format = GL_RED; inter = GL_RED; }
			else if (nrChannels == 2) { format = GL_RG; inter = GL_RG; }
			else if (nrChannels == 3) { format = GL_RGB; inter = GL_SRGB; }
			else if (nrChannels == 4) { format = GL_RGBA; inter = GL_SRGB_ALPHA; }

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			spdlog::error("Skybox texture failed to load at path: {}", faces[i]);
			stbi_image_free(data);
			err = true;
			break;
		}
	}

	if (err) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		Skybox::_window = nullptr;
		return;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLuint captureFBO = 0, captureRBO = 0;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// 2048 / 16
	int irrSize = 32;
	glGenTextures(1, &Skybox::_irradianceTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, irrSize, irrSize, 0, format, GL_UNSIGNED_INT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irrSize, irrSize);

	irradianceShader->Use();
	irradianceShader->SetInt("environmentMap", 0);
	irradianceShader->SetMat4("projection", captureProjection);
	irradianceShader->SetFloat("scale", 2.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glViewport(0, 0, irrSize, irrSize);
	glCullFace(GL_FRONT);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader->SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Skybox::_irradianceTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(Skybox::_vao);
		glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
	glViewport(0, 0, s.x, s.y);
	glCullFace(GL_BACK);

	delete irradianceShader;
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	Skybox::_init = true;
}

void Skybox::Init(GLFWwindow* window, const GLchar* hdr)
{
	if (Skybox::_init) {
		spdlog::info("Skybox already initialized!");
		return;
	}

	if (!(std::filesystem::path(std::string(hdr)).extension().string() == std::string(".hdr"))) {
		spdlog::error("Image was not a HDR image!");
		return;
	}

	if (window == nullptr) {
		spdlog::info("Window was nullptr!");
		return;
	}

	if (Skybox::_shader == nullptr) {
		Skybox::_shader = new Shader("./res/shader/skybox.vert", "./res/shader/skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	Skybox::_window = window;

	glGenVertexArrays(1, &Skybox::_vao);
	glBindVertexArray(Skybox::_vao);

	glBindBuffer(GL_ARRAY_BUFFER, Shape::GetCubeVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetCubeEBO());

	// Vertices positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Skybox::_hdr = true;

	Skybox::_paths[0] = hdr;

	GLuint hdrTexture = 0;
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(hdr, &width, &height, &nrChannels, 0);
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
	}
	else
	{
		stbi_image_free(data);

		Skybox::_paths[0] = "";

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		spdlog::error("Skybox equirectangular texture failed to load at path: {}", hdr);
		return;
	}

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
	//int size = std::max(s.x, s.y);
	int size = 2048;

	Shader* equirectangularShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/equirectangular.frag");

	if (!equirectangularShader->IsInitialized())
	{
		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0] = "";

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		spdlog::error("Skybox equirectangular shader could not be initialized!");
		return;
	}

	Shader* irradianceShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0] = "";

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	GLuint captureFBO = 0, captureRBO = 0;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	equirectangularShader->Use();
	equirectangularShader->SetInt("equirectangularMap", 0);
	equirectangularShader->SetMat4("projection", captureProjection);
	equirectangularShader->SetFloat("scale", 2.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glViewport(0, 0, size, size);
	glCullFace(GL_FRONT);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularShader->SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Skybox::_texture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(Skybox::_vao);
		glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
		glBindVertexArray(0);
	}

	//size / 16
	int irrSize = 32;
	glGenTextures(1, &Skybox::_irradianceTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, irrSize, irrSize, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irrSize, irrSize);

	irradianceShader->Use();
	irradianceShader->SetInt("environmentMap", 0);
	irradianceShader->SetMat4("projection", captureProjection);
	irradianceShader->SetFloat("scale", 2.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glViewport(0, 0, irrSize, irrSize);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader->SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Skybox::_irradianceTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(Skybox::_vao);
		glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, s.x, s.y);
	glCullFace(GL_BACK);

	delete equirectangularShader;
	delete irradianceShader;
	glDeleteTextures(1, &hdrTexture);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	Skybox::_init = true;
}

void Skybox::Draw()
{
	if (!Skybox::_init) {
		spdlog::error("Skybox wasn't initialized!");
		return;
	}

	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	Skybox::_shader->Use();
	Skybox::_shader->SetInt("skybox", 0);
	glBindVertexArray(Skybox::_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
	glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
	glBindVertexArray(0);
}

void Skybox::Deinit()
{
	if (Skybox::_init) {
		if (Skybox::_hdr) {
			delete Skybox::_shader;
			Skybox::_shader = nullptr;

			glDeleteVertexArrays(1, &Skybox::_vao);
			Skybox::_vao = 0;

			glDeleteTextures(1, &Skybox::_texture);
			Skybox::_texture = 0;
		}
		else {
			delete Skybox::_shader;
			Skybox::_shader = nullptr;

			glDeleteVertexArrays(1, &Skybox::_vao);
			Skybox::_vao = 0;

			glDeleteTextures(1, &Skybox::_texture);
			Skybox::_texture = 0;
		}
	}
}

void Skybox::UseTexture(unsigned int samplerId)
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_2D, Skybox::_texture);
}

void Skybox::UseIrradianceTexture(unsigned int samplerId)
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_2D, Skybox::_irradianceTexture);
}