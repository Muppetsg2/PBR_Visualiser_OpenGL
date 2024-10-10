#include <Skybox.h>
#include <Shape.h>

GLuint Skybox::_vao = 0;
GLuint Skybox::_texture = 0;
Shader* Skybox::_shader = nullptr;

bool Skybox::_init = false;
bool Skybox::_hdri = false;

const GLchar* Skybox::paths[6] = { "", "", "", "", "", "" };

void Skybox::Init(const GLchar* faces[6])
{
	if (Skybox::_init) {
		spdlog::info("Skybox already initialized!");
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
		unsigned char* data = isHDR ? reinterpret_cast<unsigned char*>(stbi_loadf(faces[i], &width, &height, &nrChannels, 0)) : stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum inter = GL_SRGB;
			GLenum format = GL_RGB;
			if (nrChannels == 1) { format = GL_RED; inter = isHDR ? GL_R32F : GL_RED; }
			else if (nrChannels == 2) { format = GL_RG; inter = isHDR ? GL_RG32F : GL_RG; }
			else if (nrChannels == 3) { format = GL_RGB; inter = isHDR ? GL_RGB32F : GL_SRGB; }
			else if (nrChannels == 4) { format = GL_RGBA; inter = isHDR ? GL_RGBA32F : GL_SRGB_ALPHA; }

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, width, height, 0, format, isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			spdlog::error("Skybox texture failed to load at path: %s", faces[i]);
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
		return;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	Skybox::_init = true;
}

void Skybox::Init(const GLchar* hdr)
{
	if (Skybox::_init) {
		spdlog::info("Skybox already initialized!");
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

	if (!panSh->IsInitialized()) {
		delete panSh;
		panSh = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;

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

		spdlog::error("Image was not a HDR image!");
		return;
	}


	GLuint panorama = 0;

	glGenTextures(1, &panorama);
	glBindTexture(GL_TEXTURE_2D, panorama);

	int width, height, nrChannels;
	GLenum inter = GL_RGB32F;
	GLenum format = GL_RGB;
	unsigned char* data = reinterpret_cast<unsigned char*>(stbi_loadf(hdr, &width, &height, &nrChannels, 0));
	if (data)
	{
		if (nrChannels == 1) { format = GL_RED; inter = GL_R32F; }
		else if (nrChannels == 2) { format = GL_RG; inter = GL_RG32F; }
		else if (nrChannels == 3) { format = GL_RGB; inter = GL_RGB32F; }
		else if (nrChannels == 4) { format = GL_RGBA; inter = GL_RGBA32F; }

		glTexImage2D(GL_TEXTURE_2D, 0, inter, width, height, 0, format, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		spdlog::error("Skybox panorama texture failed to load at path: %s", hdr);
		stbi_image_free(data);

		glDeleteTextures(1, &panorama);
		panorama = 0;

		delete panSh;
		panSh = nullptr;

		delete Skybox::_shader;
		Skybox::_shader = nullptr;

		glDeleteVertexArrays(1, &Skybox::_vao);
		Skybox::_vao = 0;
		return;
	}

	glGenTextures(1, &Skybox::_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox::_texture);

	/*
	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inter, this->width, this->height, 0, format, GL_FLOAT, nullptr);
	}

	gl_filter_min = (with_mipmaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	gl_filter_mag = GL_LINEAR;

	gl_wrap_r = GL_CLAMP_TO_EDGE;
	gl_wrap_s = GL_CLAMP_TO_EDGE;
	gl_wrap_t = GL_CLAMP_TO_EDGE;

	glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, gl_filter_mag);

	glTexParameteri(gl_target, GL_TEXTURE_WRAP_R, gl_wrap_r);
	glTexParameteri(gl_target, GL_TEXTURE_WRAP_S, gl_wrap_s);
	glTexParameteri(gl_target, GL_TEXTURE_WRAP_T, gl_wrap_t);

	spdlog::debug("[Texture][{}] created empty {} [{}:{}]", name, magic_enum::enum_name(this->m_type), width, height);
	initalized = true;

	spdlog::debug("[IblSampler] panorama_to_cubemap()");
	assert(cubemap_texture.isInitalized());
	auto shaderman = &Shaderman::getInstance();

	glBindVertexArray(1);

	for (int i = 0; i < 6; ++i) {

		framebuffer.bind();
		int side = i;
		reset_gl_error();
		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,
			cubemap_texture.getGlTexture(),
			0);

		assert_gl_error();

		cubemap_texture.bind();

		glViewport(0, 0, texture_size, texture_size);
		glClearColor(0.5f, 0.5f, 0.5f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		assert_gl_error();
		auto shader = shaderman->get_named_shader(SHADER_NAME_PANORAMA_TO_CUBEMAP);
		glUseProgram(shader->program());
		assert_gl_error();

		//input_texture.bind();
		glActiveTexture(GL_TEXTURE0 + input_texture.getTextureUnit());
		glBindTexture(input_texture.getGlTarget(), input_texture.getGlTexture());
		assert_gl_error();

		shader->upload_uniform_int("u_panorama", input_texture.getTextureUnit());
		shader->upload_uniform_int("u_currentFace", i);
		assert_gl_error();


		glDrawArrays(GL_TRIANGLES, 0, 3);
		assert_gl_error();
	}

	cubemap_texture.bind();
	cubemap_texture.generate_mipmap();
	*/
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
		if (Skybox::_hdri) {

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