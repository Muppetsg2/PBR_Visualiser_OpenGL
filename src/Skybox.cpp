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

	Skybox::_hdri = false;

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

void Skybox::Init(const GLchar* hdri)
{

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