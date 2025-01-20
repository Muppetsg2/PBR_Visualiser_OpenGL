#include <Skybox.h>
#include <Shape.h>
#if WINDOW_APP
#include <Texture2D.h>

Shader* Skybox::_shader = nullptr;
#endif
GLuint Skybox::_vao = 0;
GLuint Skybox::_texture = 0;

GLuint Skybox::_irradianceTexture = 0;
GLuint Skybox::_prefilterTexture = 0;
GLuint Skybox::_brdfLUTTexture = 0;

bool Skybox::_init = false;
bool Skybox::_hdr = false;

float Skybox::_exposure = 1.0f;
float Skybox::_colorIntensity = 1.0f;

#if WINDOW_APP
SkyboxDisplay Skybox::_displayMode = SkyboxDisplay::DEFAULT;
float Skybox::_mipmapLevel = 0.0f;
float Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = 0.f;
bool Skybox::_fromData = false;
bool Skybox::_openImageDialogs[8] = { false, false, false, false, false, false, false, false };
ImFileDialogInfo Skybox::_imageDialogInfos[8];
#endif

std::string Skybox::_paths[6] = { "", "", "", "", "", "" };
GLFWwindow* Skybox::_window = nullptr;
glm::ivec2 Skybox::_windowSize = glm::ivec2();

bool Skybox::GenerateBRDFLut(GLuint framebuffer, GLuint renderbuffer)
{
	Shader* brdfShader = Shader::FromExtractor("brdf.vert", "brdf.frag");

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

	glViewport(0, 0, Skybox::_windowSize.x, Skybox::_windowSize.y);

	return true;
}

std::pair<bool, std::string> Skybox::CheckFolder()
{
	std::string path = std::filesystem::absolute(Skybox::_paths[0]).string();

	std::string name;
	if (Skybox::_hdr)
	{
		name = std::filesystem::path(path).filename().replace_extension("").string().append("_data");
	}
	else {
		name = path.substr(0, path.find_last_of("\\/"));
		name = name.substr(name.find_last_of("\\/") + 1).append("_data");
	}
	std::string folderPath = path.substr(0, path.find_last_of("\\/") + 1).append(name);

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
	if (Config::isVerbose()) spdlog::info("Cubemap saving started!");

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
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB, GL_FLOAT, mipData.data());
				glPixelStorei(GL_PACK_ALIGNMENT, 4);

				std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(float));

				mipData.clear();
			}
		}

		if (gli::save_dds(texCube, dir + "\\cubemap.dds")) {
			if (Config::isVerbose()) spdlog::info("File 'cubemap.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'cubemap.dds' in directory '{}'", dir);
		}
	}
	else
	{
		GLint inter, nrChannels = 3, format = GL_RGB;
		gli::format gFormat = gli::FORMAT_RGB8_SRGB_PACK8;

		// Save Cubemap
		glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_INTERNAL_FORMAT, &inter);

		maxMipLevel = 1 + floor(log2(std::max(width, height)));

		switch (inter) {
			case GL_RED:
				format = GL_R;
				nrChannels = 1;
				gFormat = gli::FORMAT_RGBA8_SRGB_PACK8;
				break;
			case GL_RG:
				format = GL_RG;
				nrChannels = 2;
				gFormat = gli::FORMAT_RGBA8_SRGB_PACK8;
				break;
			case GL_SRGB:
				format = GL_RGB;
				nrChannels = 3;
				gFormat = gli::FORMAT_RGBA8_SRGB_PACK8;
				break;
			case GL_SRGB_ALPHA:
				format = GL_RGBA;
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
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, GL_UNSIGNED_BYTE, mipData.data());
				glPixelStorei(GL_PACK_ALIGNMENT, 4);

				if (nrChannels < 4) {
					std::vector<unsigned char> newImage(mipWidth * mipHeight * 4);

					for (int i = 0; i < mipWidth * mipHeight; ++i) {
						int oldIndex = i * nrChannels;
						int newIndex = i * 4;

						std::memcpy(&newImage[newIndex], &mipData[oldIndex], sizeof(unsigned char) * nrChannels);

						if (nrChannels < 3) {
							std::memset(&newImage[newIndex + nrChannels], 0, sizeof(unsigned char) * (3 - nrChannels));
						}

						newImage[newIndex + 3] = 255;
					}

					std::memcpy(texCube[face][level].data(), newImage.data(), newImage.size() * sizeof(unsigned char));
				}
				else {
					std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(unsigned char));
				}
			}
		}

		if (gli::save_dds(texCube, dir + "\\cubemap.dds")) {
			if (Config::isVerbose()) spdlog::info("File 'cubemap.dds' saved in directory '{}'", dir);
		}
		else {
			spdlog::error("There was an error while trying to save 'cubemap.dds' in directory '{}'", dir);
		}
	}

	// Save IRRADIANCE
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

	gli::texture_cube texCube = gli::texture_cube(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent3d(width, height, 1), 1);

	for (int face = 0; face < 6; ++face) {
		std::vector<float> mipData(width * height * 3); // GL_RGB32F
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, mipData.data());
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		std::memcpy(texCube[face].data(), mipData.data(), mipData.size() * sizeof(float));

		mipData.clear();
	}

	if (gli::save_dds(texCube, dir + "\\irradiance.dds")) {
		if (Config::isVerbose()) spdlog::info("File 'irradiance.dds' saved in directory '{}'", dir);
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
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB, GL_FLOAT, mipData.data());
			glPixelStorei(GL_PACK_ALIGNMENT, 4);

			std::memcpy(texCube[face][level].data(), mipData.data(), mipData.size() * sizeof(float));

			mipData.clear();
		}
	}

	if (gli::save_dds(texCube, dir + "\\prefilter.dds")) {
		if (Config::isVerbose()) spdlog::info("File 'prefilter.dds' saved in directory '{}'", dir);
	}
	else {
		spdlog::error("There was an error while trying to save 'prefilter.dds' in directory '{}'", dir);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// Save BRDFLut
	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	float* data = new float[width * height * 2]; // GL_RG32F
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, data);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	stbi_flip_vertically_on_write(true);
	int result = stbi_write_hdr(std::string(dir + "\\brdfLUT.hdr").c_str(), width, height, 2, data);

	if (result != 0) {
		if (Config::isVerbose()) spdlog::info("File 'brdfLUT.hdr' saved in directory '{}'", dir);
	}
	else {
		spdlog::error("There was an error while trying to save 'brdfLUT.hdr' in directory '{}'", dir);
	}

	delete[] data;

	glBindTexture(GL_TEXTURE_2D, 0);

	if (Config::isVerbose()) spdlog::info("Cubemap saving ended!");
}

bool Skybox::LoadSavedData(std::string dir)
{
	gli::texture_cube texCube = (gli::texture_cube)gli::load_dds(dir + "\\cubemap.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'cubemap.dds' from directory: {}", dir);
		return false;
	}

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format Format = GL.translate(texCube.format(), texCube.swizzles());

	GLenum target = GL.translate(texCube.target());
	glGenTextures(1, &Skybox::_texture);
	glBindTexture(target, Skybox::_texture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

#if WINDOW_APP
	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = texCube.levels() - 1;
#endif

	gli::ivec2 extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	Skybox::_hdr = GL_RGB32F == (int)Format.Internal;

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		for (std::size_t level = 0; level < texCube.levels(); ++level) {
			glm::ivec2 levelExtent(texCube.extent(level));
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				(GLint)level,
				0, 0,
				levelExtent.x, levelExtent.y,
				Format.External, Format.Type,
				texCube.data(0, face, level)
			);
		}
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Cubemap Texture loaded!");

	texCube.clear();

	texCube = (gli::texture_cube)gli::load_dds(dir + "\\irradiance.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'irradiance.dds' from directory: {}", dir);

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		return false;
	}

	Format = GL.translate(texCube.format(), texCube.swizzles());

	target = GL.translate(texCube.target());
	glGenTextures(1, &Skybox::_irradianceTexture);
	glBindTexture(target, Skybox::_irradianceTexture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

	extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		glTexSubImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0, 0, 0,
			extent.x, extent.y,
			Format.External, Format.Type,
			texCube.data(0, face, 0)
		);
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Irradiance Texture loaded!");

	texCube.clear();

	texCube = (gli::texture_cube)gli::load_dds(dir + "\\prefilter.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'prefilter.dds' from directory: {}", dir);

		glDeleteTextures(1, &Skybox::_irradianceTexture);
		Skybox::_irradianceTexture = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		return false;
	}

	Format = GL.translate(texCube.format(), texCube.swizzles());

	target = GL.translate(texCube.target());
	glGenTextures(1, &Skybox::_prefilterTexture);
	glBindTexture(target, Skybox::_prefilterTexture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

	extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		for (std::size_t level = 0; level < texCube.levels(); ++level) {
			glm::ivec2 levelExtent(texCube.extent(level));
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				(GLint)level,
				0, 0,
				levelExtent.x, levelExtent.y,
				Format.External, Format.Type,
				texCube.data(0, face, level)
			);
		}
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Prefilter Texture loaded!");

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(std::string(dir + "\\brdfLUT.hdr").c_str(), &width, &height, &channels, 0);

	if (!data) {
		spdlog::error("Error occured while trying to load HDR image 'brdfLUT.hdr' in directory: {}", dir);

		glDeleteTextures(1, &Skybox::_prefilterTexture);
		Skybox::_prefilterTexture = 0;

		glDeleteTextures(1, &Skybox::_irradianceTexture);
		Skybox::_irradianceTexture = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		stbi_image_free(data);
		return false;
	}

	int new_channels = 2;  // RG
	std::vector<float> data_RG(width * height * new_channels);

	for (int i = 0; i < width * height; ++i) {
		data_RG[i * new_channels] = data[i * channels];         // R
		data_RG[i * new_channels + 1] = data[i * channels + 1]; // G
	}

	stbi_image_free(data);

	glGenTextures(1, &Skybox::_brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, Skybox::_brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, data_RG.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data_RG.clear();

	glBindTexture(GL_TEXTURE_2D, 0);
	
	if (Config::isVerbose()) spdlog::info("BRDFLut Texture loaded!");

	return true;
}

bool Skybox::LoadSavedDataToChange(std::string dir)
{
	gli::texture_cube texCube = (gli::texture_cube)gli::load_dds(dir + "\\cubemap.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'cubemap.dds' from directory: {}", dir);
		return false;
	}

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format Format = GL.translate(texCube.format(), texCube.swizzles());

	glDeleteTextures(1, &Skybox::_texture);
	glGenTextures(1, &Skybox::_texture);

	GLenum target = GL.translate(texCube.target());
	glBindTexture(target, Skybox::_texture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

#if WINDOW_APP
	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = texCube.levels() - 1;
#endif

	gli::ivec2 extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	Skybox::_hdr = GL_RGB32F == (int)Format.Internal;

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		for (std::size_t level = 0; level < texCube.levels(); ++level) {
			glm::ivec2 levelExtent(texCube.extent(level));
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				(GLint)level,
				0, 0,
				levelExtent.x, levelExtent.y,
				Format.External, Format.Type,
				texCube.data(0, face, level)
			);
		}
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Cubemap Texture loaded!");

	texCube.clear();

	texCube = (gli::texture_cube)gli::load_dds(dir + "\\irradiance.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'irradiance.dds' from directory: {}", dir);

		return false;
	}

	Format = GL.translate(texCube.format(), texCube.swizzles());

	glDeleteTextures(1, &Skybox::_irradianceTexture);
	glGenTextures(1, &Skybox::_irradianceTexture);

	target = GL.translate(texCube.target());
	glBindTexture(target, Skybox::_irradianceTexture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

	extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		glTexSubImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0, 0, 0,
			extent.x, extent.y,
			Format.External, Format.Type,
			texCube.data(0, face, 0)
		);
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Irradiance Texture loaded!");

	texCube.clear();

	texCube = (gli::texture_cube)gli::load_dds(dir + "\\prefilter.dds");
	if (texCube.empty()) {
		spdlog::error("An error occurred while reading file 'prefilter.dds' from directory: {}", dir);

		return false;
	}

	Format = GL.translate(texCube.format(), texCube.swizzles());

	glDeleteTextures(1, &Skybox::_prefilterTexture);
	glGenTextures(1, &Skybox::_prefilterTexture);

	target = GL.translate(texCube.target());
	glBindTexture(target, Skybox::_prefilterTexture);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, (GLint)(texCube.levels() - 1));

	extent = texCube.extent();
	glTexStorage2D(target, (GLint)texCube.levels(), Format.Internal, extent.x, extent.y);

	for (std::size_t face = 0; face < texCube.faces(); ++face) {
		for (std::size_t level = 0; level < texCube.levels(); ++level) {
			glm::ivec2 levelExtent(texCube.extent(level));
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				(GLint)level,
				0, 0,
				levelExtent.x, levelExtent.y,
				Format.External, Format.Type,
				texCube.data(0, face, level)
			);
		}
	}

	glBindTexture(target, 0);

	if (Config::isVerbose()) spdlog::info("Prefilter Texture loaded!");

	for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

	Skybox::_paths[0] = dir;

#if WINDOW_APP
	Skybox::_fromData = true;
#endif
	return true;
}

void Skybox::Init(glm::ivec2 window_size, const GLchar* hdr)
{
	if (Skybox::_init) {
		if (Config::isVerbose()) spdlog::info("Skybox already initialized!");
		return;
	}

	if (!(std::filesystem::path(std::string(hdr)).extension().string() == std::string(".hdr"))) {
		spdlog::error("Image was not a HDR image!");
		return;
	}

#if WINDOW_APP
	if (Skybox::_shader == nullptr) {
		Skybox::_shader = Shader::FromExtractor("skybox.vert", "skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox shader initialized!");
#endif

	Skybox::_windowSize = window_size;

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
		if (Skybox::LoadSavedData(res.second)) Skybox::_init = true; return;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif
		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		if (Config::isVerbose()) spdlog::info("Directory '{}' has been created!", res.second);
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

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		spdlog::error("Skybox equirectangular texture failed to load at path: {}", hdr);
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox equirectangular texture loaded at path: {}", hdr);

	int size = 2048;

	Shader* equirectangularShader = Shader::FromExtractor("equirectangular.vert", "equirectangular.frag");

	if (!equirectangularShader->IsInitialized())
	{
		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		spdlog::error("Skybox equirectangular shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox equirectangular shader initialized!");

	Shader* irradianceShader = Shader::FromExtractor("equirectangular.vert", "irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox irradiance shader initialized!");

	Shader* prefilterShader = Shader::FromExtractor("equirectangular.vert", "prefilter.frag");

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

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox prefilter shader initialized!");

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

#if WINDOW_APP
	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = floor(log2(size));
#endif

	if (Config::isVerbose()) spdlog::info("Cubemap Texture generated!");

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

	if (Config::isVerbose()) spdlog::info("Irradiance Texture generated!");

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

	if (Config::isVerbose()) spdlog::info("Prefilter Texture generated!");

	if (!GenerateBRDFLut(captureFBO, captureRBO))
	{
		spdlog::error("BRDFLut Texture couldn't have been generated!");
		delete prefilterShader;

		delete irradianceShader;

		delete equirectangularShader;

		glDeleteTextures(1, &hdrTexture);

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);

		glViewport(0, 0, window_size.x, window_size.y);
		glCullFace(GL_BACK);

		return;
	}

	if (Config::isVerbose()) spdlog::info("BRDFLut Texture generated!");

	glViewport(0, 0, window_size.x, window_size.y);
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

void Skybox::Init(GLFWwindow* window, const GLchar* hdr)
{
	if (Skybox::_init) {
		if (Config::isVerbose()) spdlog::info("Skybox already initialized!");
		return;
	}

	if (window == nullptr) {
		spdlog::info("Window was nullptr!");
		return;
	}

	Skybox::_window = window;

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);

	Skybox::Init(s, hdr);
}

#if WINDOW_APP
void Skybox::Init(glm::ivec2 window_size, const GLchar* faces[6])
{
	if (Skybox::_init) {
		if (Config::isVerbose()) spdlog::info("Skybox already initialized!");
		return;
	}

	if (Skybox::_shader == nullptr) {
		Skybox::_shader = Shader::FromExtractor("skybox.vert", "skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox shader initialized!");

	Skybox::_windowSize = window_size;

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

	for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i] = faces[i];

	std::pair<bool, std::string> res = Skybox::CheckFolder();

	if (res.first) {
		if (Skybox::LoadSavedData(res.second)) Skybox::_init = true; return;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();
		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		if (Config::isVerbose()) spdlog::info("Directory '{}' has been created!", res.second);
		isDir = true;
	}
	else {
		spdlog::error("Directory '{}' couldn't have been created!", res.second);
	}

	glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	bool err = false;
	int width, height, nrChannels;
	GLenum inter = GL_SRGB;
	GLenum format = GL_RGB;
	for (unsigned int i = 0; i < 6; ++i)
	{
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			if (nrChannels == 1) { format = GL_RED; inter = GL_RED; }
			else if (nrChannels == 2) { format = GL_RG; inter = GL_RG; }
			else if (nrChannels == 3) { format = GL_RGB; inter = GL_SRGB; }
			else if (nrChannels == 4) { format = GL_RGBA; inter = GL_SRGB_ALPHA; }

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);

			if (Config::isVerbose()) spdlog::info("Skybox texture loaded at path: {}", faces[i]);
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
		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();
		return;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = floor(log2(std::max(width, height)));

	Shader* irradianceShader = Shader::FromExtractor("equirectangular.vert", "irradiance.frag");

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
		Skybox::_windowSize = glm::ivec2();

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox irradiance shader initialized!");

	Shader* prefilterShader = Shader::FromExtractor("equirectangular.vert", "prefilter.frag");

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
		Skybox::_windowSize = glm::ivec2();

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox prefilter shader initialized!");

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

	if (Config::isVerbose()) spdlog::info("Irradiance Texture generated!");

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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(Skybox::_vao);
			glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
			glBindVertexArray(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (Config::isVerbose()) spdlog::info("Prefilter Texture generated!");

	if (!GenerateBRDFLut(captureFBO, captureRBO))
	{
		spdlog::error("BRDFLut Texture couldn't have been generated!");
		delete prefilterShader;

		delete irradianceShader;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;

		glDeleteTextures(1, &Skybox::_irradianceTexture);
		Skybox::_irradianceTexture = 0;

		glDeleteTextures(1, &Skybox::_prefilterTexture);
		Skybox::_prefilterTexture = 0;

		Skybox::_window = nullptr;
		Skybox::_windowSize = glm::ivec2();

		glViewport(0, 0, window_size.x, window_size.y);
		glCullFace(GL_BACK);

		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (Config::isVerbose()) spdlog::info("BRDFLut Texture generated!");

	glViewport(0, 0, window_size.x, window_size.y);
	glCullFace(GL_BACK);

	delete irradianceShader;
	delete prefilterShader;
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	if (isDir) Skybox::SaveData(res.second);

	Skybox::_init = true;
}

void Skybox::Init(GLFWwindow* window, const GLchar* faces[6])
{
	if (Skybox::_init) {
		if (Config::isVerbose()) spdlog::info("Skybox already initialized!");
		return;
	}

	if (window == nullptr) {
		spdlog::info("Window was nullptr!");
		return;
	}

	Skybox::_window = window;

	glm::ivec2 s{};
	glfwGetWindowSize(Skybox::_window, &s.x, &s.y);

	Skybox::Init(s, faces);
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
	Skybox::_shader->SetFloat("exposure", _exposure);
	Skybox::_shader->SetFloat("colorIntensity", _colorIntensity);
#if WINDOW_APP
	switch (Skybox::_displayMode) {
		case SkyboxDisplay::DEFAULT: {
			Skybox::_shader->SetBool("withMipmap", true);
			Skybox::_shader->SetFloat("mipmap", std::clamp(Skybox::_mipmapLevel, 0.0f, _MAX_MIPMAP_LEVEL_DEFAULT));
			break;
		}
		case SkyboxDisplay::PREFILTER: {
			Skybox::_shader->SetBool("withMipmap", true);
			Skybox::_shader->SetFloat("mipmap", std::clamp(Skybox::_mipmapLevel, 0.0f, 9.0f));
			break;
		}
		case SkyboxDisplay::IRRADIANCE:
		default: {
			Skybox::_shader->SetBool("withMipmap", false);
			Skybox::_shader->SetFloat("mipmap", 0.0);
			break;
		}
	}
#else
	Skybox::_shader->SetBool("withMipmap", false);
	Skybox::_shader->SetFloat("mipmap", 0.0);
#endif
	glBindVertexArray(Skybox::_vao);
	glActiveTexture(GL_TEXTURE0);
#if WINDOW_APP
	switch (Skybox::_displayMode) {
		case SkyboxDisplay::IRRADIANCE: {
			glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_irradianceTexture);
			break;
		}
		case SkyboxDisplay::PREFILTER: {
			glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_prefilterTexture);
			break;
		}
		case SkyboxDisplay::DEFAULT:
		default: {
			glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
			break;
		}
	}
#else
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
#endif
	glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
	glBindVertexArray(0);
}
#endif

void Skybox::Deinit()
{
	if (Skybox::_init) {
#if WINDOW_APP
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
#endif

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

		glDeleteTextures(1, &Skybox::_texture);
		Skybox::_texture = 0;
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

float Skybox::GetExposure()
{
	return _exposure;
}

float Skybox::GetColorIntensity()
{
	return _colorIntensity;
}

#if WINDOW_APP
SkyboxDisplay Skybox::GetSkyboxDisplay()
{
	return Skybox::_displayMode;
}

float Skybox::GetMipMapLevel()
{
	return Skybox::_mipmapLevel;
}

void Skybox::SetSkyboxDisplay(SkyboxDisplay mode)
{
	Skybox::_displayMode = mode;
}

void Skybox::SetMipMapLevel(float value)
{
	Skybox::_mipmapLevel = std::clamp(value, 0.f, FLT_MAX);
}
#endif

void Skybox::SetExposure(float value)
{
	if (_exposure == value) return;

	_exposure = value;
}

void Skybox::SetColorIntensity(float value)
{
	if (_colorIntensity == value) return;

	_colorIntensity = value;
}

void Skybox::ChangeTexture(const GLchar* hdr)
{
	if (!Skybox::_init) {
		spdlog::error("Skybox wasn't initialized!");
		return;
	}

	if (!(std::filesystem::path(std::string(hdr)).extension().string() == std::string(".hdr"))) {
		spdlog::error("Image was not a HDR image!");
		return;
	}

	if (std::filesystem::path(std::string(hdr)).string() == Skybox::_paths[0]) {
		if (Config::isVerbose()) spdlog::info("Skybox already loaded!");
		return;
	}

#if WINDOW_APP
	if (Skybox::_shader == nullptr) {
		Skybox::_shader = Shader::FromExtractor("skybox.vert", "skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox shader initialized!");
#endif

	bool oldHDR = Skybox::_hdr;


	if (!oldHDR) {
		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		glDeleteTextures(1, &Skybox::_texture);
	}

	Skybox::_hdr = true;
	Skybox::_paths[0] = hdr;

	std::pair<bool, std::string> res = Skybox::CheckFolder();

	if (res.first) {
		if (Skybox::LoadSavedDataToChange(res.second)) return;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		if (Config::isVerbose()) spdlog::info("Directory '{}' has been created!", res.second);
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

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		spdlog::error("Skybox equirectangular texture failed to load at path: {}", hdr);
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox equirectangular texture loaded at path: {}", hdr);

	int size = 2048;

	Shader* equirectangularShader = Shader::FromExtractor("equirectangular.vert", "equirectangular.frag");

	if (!equirectangularShader->IsInitialized())
	{
		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		spdlog::error("Skybox equirectangular shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox equirectangular shader initialized!");

	Shader* irradianceShader = Shader::FromExtractor("equirectangular.vert", "irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		delete equirectangularShader;
		equirectangularShader = nullptr;

		glDeleteTextures(1, &hdrTexture);
		hdrTexture = 0;

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox irradiance shader initialized!");

	Shader* prefilterShader = Shader::FromExtractor("equirectangular.vert", "prefilter.frag");

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

		Skybox::_paths[0].clear();

		Skybox::_hdr = false;

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox prefilter shader initialized!");

	// CUBEMAP
	GLuint captureFBO = 0, captureRBO = 0;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	if (!oldHDR) glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);
	if (!oldHDR) {
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
		}
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

#if WINDOW_APP
	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = floor(log2(size));
#endif

	if (Config::isVerbose()) spdlog::info("Cubemap Texture generated!");

	// IRRADIANCE
	//size / 16
	int irrSize = 32;
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

	if (Config::isVerbose()) spdlog::info("Irradiance Texture generated!");

	// PREFILTER
	int preSize = 512;
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

	if (Config::isVerbose()) spdlog::info("Prefilter Texture generated!");

	glViewport(0, 0, Skybox::_windowSize.x, Skybox::_windowSize.y);
	glCullFace(GL_BACK);

	delete equirectangularShader;
	delete irradianceShader;
	delete prefilterShader;
	glDeleteTextures(1, &hdrTexture);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	if (isDir) Skybox::SaveData(res.second);
}

#if WINDOW_APP
void Skybox::ChangeTexture(const GLchar* faces[6])
{
	if (!Skybox::_init) {
		spdlog::error("Skybox wasn't initialized!");
		return;
	}

	if (Skybox::_shader == nullptr) {
		Skybox::_shader = Shader::FromExtractor("skybox.vert", "skybox.frag");
	}

	if (!Skybox::_shader->IsInitialized()) {
		delete Skybox::_shader;
		Skybox::_shader = nullptr;
		spdlog::error("Skybox shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox shader initialized!");

	bool oldHDR = Skybox::_hdr;

	Skybox::_hdr = false;

	for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i] = faces[i];

	std::pair<bool, std::string> res = Skybox::CheckFolder();

	if (res.first) {
		if (Skybox::LoadSavedDataToChange(res.second)) return;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();
		return;
	}

	bool isDir = false;

	if (_mkdir(res.second.c_str()) == 0) {
		if (Config::isVerbose()) spdlog::info("Directory '{}' has been created!", res.second);
		isDir = true;
	}
	else {
		spdlog::error("Directory '{}' couldn't have been created!", res.second);
	}

	if (oldHDR) {
		glDeleteTextures(1, &Skybox::_texture);
		glGenTextures(1, &Skybox::_texture);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	bool err = false;
	int width, height, nrChannels;
	GLenum inter = GL_SRGB;
	GLenum format = GL_RGB;
	for (unsigned int i = 0; i < 6; ++i)
	{
		unsigned char* data = stbi_load(Skybox::_paths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			if (nrChannels == 1) { format = GL_RED; inter = GL_RED; }
			else if (nrChannels == 2) { format = GL_RG; inter = GL_RG; }
			else if (nrChannels == 3) { format = GL_RGB; inter = GL_SRGB; }
			else if (nrChannels == 4) { format = GL_RGBA; inter = GL_SRGB_ALPHA; }

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);

			if (Config::isVerbose()) spdlog::info("Skybox texture loaded at path: {}", Skybox::_paths[i]);
		}
		else
		{
			spdlog::error("Skybox texture failed to load at path: {}", Skybox::_paths[i]);
			stbi_image_free(data);
			err = true;
			break;
		}
	}

	if (err) {
		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();
		return;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	Skybox::_MAX_MIPMAP_LEVEL_DEFAULT = floor(log2(std::max(width, height)));

	Shader* irradianceShader = Shader::FromExtractor("equirectangular.vert", "irradiance.frag");

	if (!irradianceShader->IsInitialized())
	{
		delete irradianceShader;
		irradianceShader = nullptr;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		spdlog::error("Skybox irradiance shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox irradiance shader initialized!");

	Shader* prefilterShader = Shader::FromExtractor("equirectangular.vert", "prefilter.frag");

	if (!prefilterShader->IsInitialized())
	{
		delete prefilterShader;
		prefilterShader = nullptr;

		delete irradianceShader;
		irradianceShader = nullptr;

		for (unsigned int i = 0; i < 6; ++i) Skybox::_paths[i].clear();

		spdlog::error("Skybox prefilter shader could not be initialized!");
		return;
	}

	if (Config::isVerbose()) spdlog::info("Skybox prefilter shader initialized!");

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

	if (Config::isVerbose()) spdlog::info("Irradiance Texture generated!");

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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(Skybox::_vao);
			glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
			glBindVertexArray(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (Config::isVerbose()) spdlog::info("Prefilter Texture generated!");

	glViewport(0, 0, Skybox::_windowSize.x, Skybox::_windowSize.y);
	glCullFace(GL_BACK);

	delete irradianceShader;
	delete prefilterShader;
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	if (isDir) Skybox::SaveData(res.second);
}

void Skybox::DrawEditor(bool* open)
{
	if (!ImGui::Begin("Skybox", open)) {
		ImGui::End();
		return;
	}

	if (ImGui::Button("Load HDR Skybox"))
	{
		Skybox::_openImageDialogs[6] = true;
		Skybox::_imageDialogInfos[6].title = "Choose HDR image";
		Skybox::_imageDialogInfos[6].type = ImGuiFileDialogType_OpenFile;
		Skybox::_imageDialogInfos[6].directoryPath = std::filesystem::current_path().append("res\\skybox\\");
	}

	if (Skybox::_openImageDialogs[6])
	{
		if (ImGui::FileDialog(&Skybox::_openImageDialogs[6], &Skybox::_imageDialogInfos[6]))
		{
			Skybox::_fromData = false;
			ChangeTexture(Skybox::_imageDialogInfos[6].resultPath.string().c_str());
		}
	}

	static bool _facesOpen = false;

	if (ImGui::Button("Load Skybox from Faces"))
	{
		_facesOpen = true;
	}

	if (ImGui::Button("Load Skybox from Data"))
	{
		Skybox::_openImageDialogs[7] = true;
		Skybox::_imageDialogInfos[7].title = "Choose Skybox Data Folder";
		Skybox::_imageDialogInfos[7].type = ImGuiFileDialogType_OpenDirectory;
		Skybox::_imageDialogInfos[7].directoryPath = std::filesystem::current_path().append("res\\skybox\\");
	}

	if (Skybox::_openImageDialogs[7])
	{
		if (ImGui::FileDialog(&Skybox::_openImageDialogs[7], &Skybox::_imageDialogInfos[7]))
		{
			Skybox::_fromData = true;
			LoadSavedDataToChange(Skybox::_imageDialogInfos[7].resultPath.string());
		}
	}

	std::string name;
	if (Skybox::_fromData) {
		name = std::filesystem::path(Skybox::_paths[0]).filename().string();
	}
	else {
		if (_hdr) {
			name = std::filesystem::path(Skybox::_paths[0]).filename().string();
		}
		else {
			name = std::filesystem::path(Skybox::_paths[0]).string();
			name = name.substr(0, name.find_last_of("\\/"));
			name = name.substr(name.find_last_of("\\/") + 1);
		}
	}

	ImGui::Text("Name: %s", name.c_str());
	ImGui::Text("Initialized: %s", _init ? "True" : "False");
	ImGui::Text("HDR: %s", _hdr ? "True" : "False");

	if (ImGui::BeginCombo("Display Mode", to_string(Skybox::_displayMode).c_str()))
	{
		for (size_t i = 0; i < size<SkyboxDisplay>(); ++i) {
			SkyboxDisplay acc = (SkyboxDisplay)i;
			if (ImGui::Selectable(to_string(acc).c_str(), Skybox::_displayMode == acc))
			{
				Skybox::SetSkyboxDisplay(acc);
				break;
			}
		}
		ImGui::EndCombo();
	}

	if (Skybox::_displayMode == SkyboxDisplay::DEFAULT || Skybox::_displayMode == SkyboxDisplay::PREFILTER) {
		float mml = Skybox::_mipmapLevel;
		ImGui::DragFloat("MipMap Level", &mml, 0.1f, 0.0f, Skybox::_displayMode == SkyboxDisplay::PREFILTER ? 9.0f : Skybox::_MAX_MIPMAP_LEVEL_DEFAULT, "%.1f");

		if (mml != Skybox::_mipmapLevel) Skybox::SetMipMapLevel(mml);
	}

	ImGui::Text("Skybox Texture ID: %u", _texture);
	ImGui::Text("Irradiance Texture ID: %u", _irradianceTexture);
	ImGui::Text("Prefilter Texture ID: %u", _prefilterTexture);
	ImGui::Text("BRDF LUT Texture ID: %u", _brdfLUTTexture);

	if (_brdfLUTTexture) {
		ImGui::Text("BRDF LUT Texture:");
		ImGui::Image((intptr_t)(_brdfLUTTexture), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
	}

	float exp = _exposure;
	if (ImGui::SliderFloat("Exposure##Skybox", &exp, 0.0f, 11.0f, "Exposure: %.2f")) {
		SetExposure(exp);
	}

	float intensity = _colorIntensity;
	if (ImGui::SliderFloat("Color Intensity##Skybox", &intensity, 0.0f, 4.0f, "Intensity: %.2f")) {
		SetColorIntensity(intensity);
	}

	ImGui::End();

	if (_facesOpen) Skybox::DrawSkyboxFacesLoader(&_facesOpen);
}

void Skybox::DrawSkyboxFacesLoader(bool* open)
{
	if (!ImGui::Begin("Skybox Faces Loader Window", open)) {
		ImGui::End();
		return;
	}

	static const char* labels[6] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
	static Texture2D* imageTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	for (int i = 0; i < 6; ++i)
	{
		if (ImGui::Button(("Load Image " + std::string(labels[i])).c_str()))
		{
			Skybox::_openImageDialogs[i] = true;
			Skybox::_imageDialogInfos[i].title = "Choose " + std::string(labels[i]) + " image";
			Skybox::_imageDialogInfos[i].type = ImGuiFileDialogType_OpenFile;
			Skybox::_imageDialogInfos[i].directoryPath = std::filesystem::current_path().append("res\\skybox\\");
		}

		if (Skybox::_openImageDialogs[i])
		{
			if (ImGui::FileDialog(&Skybox::_openImageDialogs[i], &Skybox::_imageDialogInfos[i]))
			{
				TextureFileFormat inter = TextureFileFormat::SRGB;
				TextureFormat form = TextureFormat::RGB;
				imageTextures[i] = new Texture2D(Skybox::_imageDialogInfos[i].resultPath.string().c_str(), inter, form);
			}
		}

		if (imageTextures[i] != nullptr)
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 65);
			ImGui::Image((intptr_t)imageTextures[i]->GetId(), ImVec2(64, 64));
		}
		else
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130);
			ImGui::Text("No texture loaded");
		}
	}


	bool isDisabled = imageTextures[0] == nullptr || imageTextures[1] == nullptr || imageTextures[2] == nullptr ||
		imageTextures[3] == nullptr || imageTextures[4] == nullptr || imageTextures[5] == nullptr;

	if (isDisabled) ImGui::BeginDisabled();
	if (ImGui::Button("Confirm", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{	
		const GLchar* paths[6] = {
			strdup(imageTextures[0]->GetPath().c_str()), strdup(imageTextures[1]->GetPath().c_str()), strdup(imageTextures[2]->GetPath().c_str()),
			strdup(imageTextures[3]->GetPath().c_str()), strdup(imageTextures[4]->GetPath().c_str()), strdup(imageTextures[5]->GetPath().c_str())
		};

		Skybox::_fromData = false;
		ChangeTexture(paths);

		delete imageTextures[0];
		delete imageTextures[1];
		delete imageTextures[2];
		delete imageTextures[3];
		delete imageTextures[4];
		delete imageTextures[5];

		imageTextures[0] = nullptr;
		imageTextures[1] = nullptr;
		imageTextures[2] = nullptr;
		imageTextures[3] = nullptr;
		imageTextures[4] = nullptr;
		imageTextures[5] = nullptr;

		*open = false;
	}
	if (isDisabled) ImGui::EndDisabled();

	ImGui::End();
}
#endif