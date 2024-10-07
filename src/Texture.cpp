#include <Texture.h>

Texture::Texture(const GLchar* path, int width, int height, int channelsNum, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip)
{

}

Texture::Texture(const GLchar* path, glm::ivec2 size, int channelsNum, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip)
{
	this->_id = 0;
	glGenTextures(1, &(this->_id));
	glBindTexture(GL_TEXTURE_2D, this->_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)sWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)tWrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)minFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)magFilterMode);

	// Flip Images
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* image = stbi_load(path, &(this->_size.x), &(this->_size.y), &(this->_channelsNum), 0);
	if (image)
	{
		GLenum format{};
		if (this->_channelsNum == 1)
			format = GL_RED;
		else if (this->_channelsNum == 3)
			format = GL_RGB;
		else if (this->_channelsNum == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, _size.x, _size.y, 0, format, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::error("Failed to load texture");
	}
	stbi_image_free(image);
}

Texture::~Texture()
{
	glDeleteTextures(1, &_id);
}

void Texture::SetWrapModeS(const TextureWrapMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)mode);
	_sWrapMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetWrapModeT(const TextureWrapMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)mode);
	_tWrapMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetMinFilterMode(const TextureFilterMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
	_minFilterMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetMagFilterMode(const TextureFilterMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mode);
	_magFilterMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::GetId() const
{
	return _id;
}

glm::uvec2 Texture::GetSize() const {
	return _size;
}

unsigned int Texture::GetWidth() const
{
	return _size.x;
}

unsigned int Texture::GetHeight() const
{
	return _size.y;
}

unsigned int Texture::GetChannelsNum() const
{
	return _channelsNum;
}

TextureFormat Texture::GetFormat() const
{
	return _format;
}

TextureWrapMode Texture::GetWrapModeS() const
{
	return _sWrapMode;
}

TextureWrapMode Texture::GetWrapModeT() const
{
	return _tWrapMode;
}

TextureFilterMode Texture::GetMinFilterMode() const
{
	return _minFilterMode;
}

TextureFilterMode Texture::GetMagFilterMode() const
{
	return _magFilterMode;
}

void Texture::Use(unsigned int samplerId) const
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_2D, _id);
}