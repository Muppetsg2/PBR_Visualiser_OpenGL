#include <Skybox.h>
#include <Shape.h>
#include <Texture2D.h>

GLuint Skybox::_vao = 0;
GLuint Skybox::_texture = 0;
Shader* Skybox::_shader = nullptr;

GLuint Skybox::_irradianceTexture = 0;
GLuint Skybox::_prefilterTexture = 0;
GLuint Skybox::_brdfLUTTexture = 0;

bool Skybox::_init = false;
bool Skybox::_hdr = false;

const GLchar* Skybox::_paths[6] = { "", "", "", "", "", "" };
GLFWwindow* Skybox::_window = nullptr;

bool Skybox::GenerateBRDFLut(GLuint framebuffer, GLuint renderbuffer)
{
	Shader* brdfShader = new Shader("./res/shader/brdf.vert", "./res/shader/brdf.frag");

	if (!brdfShader->IsInitialized())
	{
		delete brdfShader;
		brdfShader = nullptr;

		spdlog::error("Skybox brdf Lut shader could not be initialized!");
		return false;
	}

	int size = 512;
	glGenTextures(1, &Skybox::_brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, size, size, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint quadVAO = 0;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, Shape::GetQuadVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetQuadEBO());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Skybox::_brdfLUTTexture, 0);

	glViewport(0, 0, size, size);
	glCullFace(GL_BACK);
	brdfShader->Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, Shape::GetQuadIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetQuadIndices());
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	delete brdfShader;
	brdfShader = nullptr;
	glDeleteVertexArrays(1, &quadVAO);

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
	glViewport(0, 0, s.x, s.y);

	return true;
}

std::pair<bool, std::string> Skybox::CheckFolder()
{
	std::string folderPath;
	if (Skybox::_hdr)
	{
		std::string path = std::filesystem::absolute(Skybox::_paths[0]).string();
		std::string name = std::filesystem::path(path).filename().replace_extension("").string().append("_data");
		folderPath = path.substr(0, path.find_last_of('\\') + 1).append(name);
	}
	else {
		std::string path = std::filesystem::absolute(Skybox::_paths[0]).string();
		std::string name = path.substr(0, path.find_last_of('\\'));
		name = name.substr(name.find_last_of('\\') + 1).append("_data");

		folderPath = path.substr(0, path.find_last_of('\\') + 1).append(name);
	}

	struct stat info;
	if (stat(folderPath.c_str(), &info) != 0) {
		return std::pair(false, folderPath);  // Folder nie istnieje
	}
	else if (info.st_mode & S_IFDIR) {
		return std::pair(true, folderPath);   // Istnieje i jest folderem
	}
	return std::pair(false, folderPath);
}

void Skybox::SaveData(std::string dir)
{
	GLint width, height, maxMipLevel;

	if (Skybox::_hdr) {

		// Save Cubemap
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		maxMipLevel = 1 + floor(log2(std::max(width, height)));

		gli::texture_cube texCube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), maxMipLevel);

		for (int face = 0; face < 6; ++face) {
			for (int level = 0; level < maxMipLevel; ++level) {

				int mipWidth = std::max(1, width >> level);
				int mipHeight = std::max(1, height >> level);

				std::vector<float> mipData(mipWidth * mipHeight * 3); // GL_RGB32F
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB, GL_FLOAT, mipData.data());

				std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(float));

				mipData.clear();
			}
		}

		if (gli::save_dds(texCube, dir + "\\cubemap.dds")) {
			spdlog::info("File 'cubemap.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'cubemap.dds' in directory '{}'", dir);
		}

		// Save IRRADIANCE
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		texCube = gli::texture_cube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), 1);

		for (int face = 0; face < 6; ++face) {
			std::vector<float> mipData(width * height * 3); // GL_RGB32F
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, mipData.data());

			std::memcpy(texCube[face].data(), mipData.data(), mipData.size() * sizeof(float));

			mipData.clear();
		}

		if (gli::save_dds(texCube, dir + "\\irradiance.dds")) {
			spdlog::info("File 'irradiance.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'irradiance.dds' in directory '{}'", dir);
		}

		// Save Prefilter
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		maxMipLevel = 1 + floor(log2(std::max(width, height)));

		texCube = gli::texture_cube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), maxMipLevel);

		for (int face = 0; face < 6; ++face) {
			for (int level = 0; level < maxMipLevel; ++level) {

				int mipWidth = std::max(1, width >> level);
				int mipHeight = std::max(1, height >> level);

				std::vector<float> mipData(mipWidth * mipHeight * 3); // GL_RGB32F
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB, GL_FLOAT, mipData.data());

				std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(float));

				mipData.clear();
			}
		}

		if (gli::save_dds(texCube, dir + "\\prefilter.dds")) {
			spdlog::info("File 'prefilter.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'prefilter.dds' in directory '{}'", dir);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	else
	{
		GLint inter, nrChannels = 3;
		gli::format gFormat = gli::FORMAT_RGB8_SRGB_PACK8;

		// Save Cubemap
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_INTERNAL_FORMAT, &inter);

		maxMipLevel = 1 + floor(log2(std::max(width, height)));

		switch (inter) {
			case GL_RED:
				nrChannels = 1;
				gFormat = gli::FORMAT_R8_SRGB_PACK8;
				break;
			case GL_RG:
				nrChannels = 2;
				gFormat = gli::FORMAT_RG8_SRGB_PACK8;
				break;
			case GL_SRGB:
				nrChannels = 3;
				gFormat = gli::FORMAT_RGB8_SRGB_PACK8;
				break;
			case GL_SRGB_ALPHA:
				nrChannels = 4;
				gFormat = gli::FORMAT_RGBA8_SRGB_PACK8;
				break;
		}

		gli::texture_cube texCube(gFormat, gli::extent3d(width, height, 1), maxMipLevel);

		for (int face = 0; face < 6; ++face) {
			for (int level = 0; level < maxMipLevel; ++level) {

				int mipWidth = std::max(1, width >> level);
				int mipHeight = std::max(1, height >> level);

				std::vector<unsigned char> mipData(mipWidth * mipHeight * nrChannels);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, inter, GL_UNSIGNED_BYTE, mipData.data());

				std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(unsigned char));
			}
		}

		if (gli::save_dds(texCube, dir + "\\cubemap.dds")) {
			spdlog::info("File 'cubemap.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'cubemap.dds' in directory '{}'", dir);
		}

		/*
		// Save IRRADIANCE
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		texCube = gli::texture_cube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), 1);

		for (int face = 0; face < 6; ++face) {
			std::vector<float> mipData(width * height * 3); // GL_RGB32F
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, mipData.data());

			std::memcpy(texCube[face].data(), mipData.data(), mipData.size() * sizeof(float));
		}

		if (gli::save_dds(texCube, dir + "\\irradiance.dds")) {
			spdlog::info("File 'irradiance.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'irradiance.dds' in directory '{}'", dir);
		}

		// Save Prefilter
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		maxMipLevel = 1 + floor(log2(std::max(width, height)));

		texCube = gli::texture_cube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), maxMipLevel);

		for (int face = 0; face < 6; ++face) {
			for (int level = 0; level < maxMipLevel; ++level) {

				int mipWidth = std::max(1, width >> level);
				int mipHeight = std::max(1, height >> level);

				std::vector<float> mipData(mipWidth * mipHeight * 3); // GL_RGB32F
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB, GL_FLOAT, mipData.data());

				std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(float));
			}
		}

		if (gli::save_dds(texCube, dir + "\\prefilter.dds")) {
			spdlog::info("File 'prefilter.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'prefilter.dds' in directory '{}'", dir);
		}

		*/
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	// Save BRDFLut
	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	float* data = new float[width * height * 2]; // GL_RG32F
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, data);

	stbi_flip_vertically_on_write(true);
	int result = stbi_write_hdr(std::string(dir + "\\brdfLUT.hdr").c_str(), width, height, 2, data);

	if (result != 0) {
		spdlog::info("File 'brdfLUT.hdr' saved in directory '{}'", dir);
	}
	else {
		spdlog::error("There was an error while trying to save 'brdfLUT.hdr' in directory '{}'", dir);
	}

	delete[] data;
}

bool Skybox::LoadSavedData(std::string dir)
{
	return true;
}

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
	GLenum inter = GL_SRGB;
	GLenum format = GL_RGB;
	for (unsigned int i = 0; i < 6; ++i)
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
		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i] = "";

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		Skybox::_window = nullptr;
		return;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	std::pair<bool, std::string> res = Skybox::CheckFolder();

	if (res.first) {
		Skybox::LoadSavedData(res.second);
		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		spdlog::info("Directory '{}' has been created!", res.second);
		isDir = true;
	}
	else {
		spdlog::error("Directory '{}' couldn't have been created!", res.second);
	}

	Shader* irradianceShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		Skybox::_window = nullptr;

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	Shader* prefilterShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/prefilter.frag");

	if (!prefilterShader->IsInitialized())
	{
		delete prefilterShader;
		prefilterShader = nullptr;

		delete irradianceShader;
		irradianceShader = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		Skybox::_window = nullptr;

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	// IRRADIANCE
	GLuint captureFBO = 0, captureRBO = 0;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

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
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, irrSize, irrSize, 0, format, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irrSize, irrSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

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

	// PREFILTER
	int preSize = 512;
	glGenTextures(1, &Skybox::_prefilterTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, preSize, preSize, 0, format, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	prefilterShader->Use();
	prefilterShader->SetInt("environmentMap", 0);
	prefilterShader->SetMat4("projection", captureProjection);
	prefilterShader->SetFloat("scale", 2.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = preSize * std::pow(0.5, mip);
		unsigned int mipHeight = preSize * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader->SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader->SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Skybox::_prefilterTexture, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(Skybox::_vao);
			glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
			glBindVertexArray(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!GenerateBRDFLut(captureFBO, captureRBO)) 
	{
		delete prefilterShader;

		delete irradianceShader;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i] = "";

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		glDeleteTextures(1, &Skybox::_irradianceTexture);
		Skybox::_irradianceTexture = 0;

		glDeleteTextures(1, &Skybox::_prefilterTexture);
		Skybox::_prefilterTexture = 0;

		Skybox::_window = nullptr;

		glm::ivec2 s{};
		glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
		glViewport(0, 0, s.x, s.y);
		glCullFace(GL_BACK);

		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);
	glViewport(0, 0, s.x, s.y);
	glCullFace(GL_BACK);

	delete irradianceShader;
	delete prefilterShader;
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	if (isDir) Skybox::SaveData(res.second);

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

	std::pair<bool, std::string> res = Skybox::CheckFolder();

	if (res.first) {
		Skybox::LoadSavedData(res.second);
		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		spdlog::info("Directory '{}' has been created!", res.second);
		isDir = true;
	}
	else {
		spdlog::error("Directory '{}' couldn't have been created!", res.second);
	}

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

	Shader* prefilterShader = new Shader("./res/shader/equirectangular.vert", "./res/shader/prefilter.frag");

	if (!prefilterShader->IsInitialized())
	{
		delete prefilterShader;
		prefilterShader = nullptr;

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

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	// CUBEMAP
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
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// IRRADIANCE
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

	// PREFILTER
	int preSize = 512;
	glGenTextures(1, &Skybox::_prefilterTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, preSize, preSize, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	prefilterShader->Use();
	prefilterShader->SetInt("environmentMap", 0);
	prefilterShader->SetMat4("projection", captureProjection);
	prefilterShader->SetFloat("scale", 2.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = preSize * std::pow(0.5, mip);
		unsigned int mipHeight = preSize * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader->SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader->SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Skybox::_prefilterTexture, mip);

			auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
				spdlog::info("Framebuffer not complete: {}\n", fboStatus);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(Skybox::_vao);
			glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
			glBindVertexArray(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!GenerateBRDFLut(captureFBO, captureRBO)) 
	{
		delete prefilterShader;

		delete irradianceShader;

		delete equirectangularShader;

		glDeleteTextures(1, &hdrTexture);

		Skybox::_paths[0] = "";

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);
		
		glViewport(0, 0, s.x, s.y);
		glCullFace(GL_BACK);

		return;
	}

	glViewport(0, 0, s.x, s.y);
	glCullFace(GL_BACK);

	delete equirectangularShader;
	delete irradianceShader;
	delete prefilterShader;
	glDeleteTextures(1, &hdrTexture);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	if (isDir) Skybox::SaveData(res.second);

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
	Skybox::_shader->SetFloat("exposure", 1.0);
	Skybox::_shader->SetBool("prefilter", false);
	Skybox::_shader->SetFloat("mipmap", 1.2);
	glBindVertexArray(Skybox::_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
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
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
}

void Skybox::UseIrradianceTexture(unsigned int samplerId)
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
}

void Skybox::UsePrefilterTexture(unsigned int samplerId)
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
}

void Skybox::UseBrdfLUTTexture(unsigned int samplerId)
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
}