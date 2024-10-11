#include <Skybox.h>
#include <Shape.h>

GLuint Skybox::_vao = 0;
GLuint Skybox::_texture = 0;
Shader* Skybox::_shader = nullptr;

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
	for (size_t i = 0; i < 6; ++i)
	{
		bool isHDR = (std::filesystem::path(std::string(faces[i])).extension().string() == std::string(".hdr"));

		Skybox::_paths[i] = faces[i];

		unsigned char* data = isHDR ? reinterpret_cast<unsigned char*>(stbi_loadf(faces[i], &width, &height, &nrChannels, 0)) : stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum inter = GL_SRGB;
			GLenum format = GL_RGB;
			if (nrChannels == 1) { format = GL_RED; inter = isHDR ? GL_R16F : GL_RED; }
			else if (nrChannels == 2) { format = GL_RG; inter = isHDR ? GL_RG16F : GL_RG; }
			else if (nrChannels == 3) { format = GL_RGB; inter = isHDR ? GL_RGB16F : GL_SRGB; }
			else if (nrChannels == 4) { format = GL_RGBA; inter = isHDR ? GL_RGBA16F : GL_SRGB_ALPHA; }

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, width, height, 0, format, isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
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

	Skybox::_init = true;
}

void Skybox::Init(GLFWwindow* window, const GLchar* hdr)
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

	Shader* panSh = new Shader("./res/shader/panorama.vert", "./res/shader/panorama.frag");

	if (!panSh->IsInitialized()) 
	{
		delete panSh;
		panSh = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		spdlog::error("Skybox panorama shader could not be initialized!");
		return;
	}

	if (!(std::filesystem::path(std::string(hdr)).extension().string() == std::string(".hdr")))
	{
		delete panSh;
		panSh = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		spdlog::error("Image was not a HDR image!");
		return;
	}

	Skybox::_paths[0] = hdr;

	GLuint panorama = 0;

	glGenTextures(1, &panorama);
	glBindTexture(GL_TEXTURE_2D, panorama);

	int width, height, nrChannels;
	GLenum inter = GL_RGB16F;
	GLenum format = GL_RGB;
	unsigned char* data = reinterpret_cast<unsigned char*>(stbi_loadf(hdr, &width, &height, &nrChannels, 0));
	if (data)
	{
		if (nrChannels == 1) { format = GL_RED; inter = GL_R16F; }
		else if (nrChannels == 2) { format = GL_RG; inter = GL_RG16F; }
		else if (nrChannels == 3) { format = GL_RGB; inter = GL_RGB16F; }
		else if (nrChannels == 4) { format = GL_RGBA; inter = GL_RGBA16F; }

		glTexImage2D(GL_TEXTURE_2D, 0, inter, width, height, 0, format, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		spdlog::error("Skybox panorama texture failed to load at path: {}", hdr);
		stbi_image_free(data);

		glDeleteTextures(1, &panorama);
		panorama = 0;

		delete panSh;
		panSh = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		return;
	}

	GLuint drawFBO = 0;
	glGenFramebuffers(1, &drawFBO);

	glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	//int size = std::max(width, height) / 4;
	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
	int size = std::max(s.x, s.y);
	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, size, size, 0, format, GL_UNSIGNED_INT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glBindVertexArray(1);

	for (int i = 0; i < 6; ++i) {

		glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);

		int side = i;

		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, Skybox::_texture, 0);

		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, size, size);

		panSh->Use();

		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, panorama);

		panSh->SetInt("u_panorama", 1);
		panSh->SetInt("u_currentFace", i);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glDeleteFramebuffers(1, &drawFBO);
	glDeleteTextures(1, &panorama);

	delete panSh;
	panSh = nullptr;

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