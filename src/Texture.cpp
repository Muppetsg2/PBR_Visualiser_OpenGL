#include <Texture.h>

void Texture::LoadTextureFromFile(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip, bool detectFormat)
{
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* image = stbi_load(path, &(this->_size.x), &(this->_size.y), &(this->_channelsNum), 0);
	if (image)
	{
		this->_path = path;

		this->_id = 0;
		glGenTextures(1, &(this->_id));
		glBindTexture(GL_TEXTURE_2D, this->_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)(this->_sWrapMode = sWrapMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)(this->_tWrapMode = tWrapMode));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)(this->_minFilterMode = minFilterMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)(this->_magFilterMode = magFilterMode));

		this->_format = format;
		this->_fileFormat = fileFormat;

		if (detectFormat) {
			if (this->_channelsNum == 1) { this->_format = TextureFormat::RED; this->_fileFormat = TextureFileFormat::RED; }
			else if (this->_channelsNum == 2) { this->_format = TextureFormat::RG; this->_fileFormat = TextureFileFormat::RG; }
			else if (this->_channelsNum == 3) { this->_format = TextureFormat::RGB; this->_fileFormat = TextureFileFormat::SRGB; }
			else if (this->_channelsNum == 4) { this->_format = TextureFormat::RGBA; this->_fileFormat = TextureFileFormat::SRGBA; }
		}

		glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)this->_fileFormat, this->_size.x, this->_size.y, 0, (GLenum)this->_format, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::error("Failed to load texture: %s\n", path);
	}

	stbi_image_free(image);
	return;
}

Texture::Texture() 
{
	_id = 0;
	_size = { 0, 0 };
	_channelsNum = 0;

	_path = "";

	_format = TextureFormat::RGB;
	_fileFormat = TextureFileFormat::SRGB;
	_sWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_tWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_minFilterMode = TextureFilterMode::NEAREST_MIPMAP_LINEAR;
	_magFilterMode = TextureFilterMode::LINEAR;
}

Texture::Texture(const Texture&& texture) 
{
	_id = texture._id;
	_size = texture._size;
	_channelsNum = texture._channelsNum;

	_path = texture._path;

	_format = texture._format;
	_fileFormat = texture._fileFormat;
	_sWrapMode = texture._sWrapMode;
	_tWrapMode = texture._tWrapMode;
	_minFilterMode = texture._minFilterMode;
	_magFilterMode = texture._magFilterMode;
}

Texture::Texture(const Texture& texture)
{
	_id = texture._id;
	_size = texture._size;
	_channelsNum = texture._channelsNum;

	_path = texture._path;

	_format = texture._format;
	_fileFormat = texture._fileFormat;
	_sWrapMode = texture._sWrapMode;
	_tWrapMode = texture._tWrapMode;
	_minFilterMode = texture._minFilterMode;
	_magFilterMode = texture._magFilterMode;
}

Texture::Texture(const GLchar* path, bool flip)
{
	LoadTextureFromFile(path, TextureFileFormat::SRGB, TextureFormat::RGB, TextureWrapMode::MIRRORED_REPEAT, TextureWrapMode::MIRRORED_REPEAT, TextureFilterMode::NEAREST_MIPMAP_LINEAR, TextureFilterMode::LINEAR, flip, true);
}

Texture::Texture(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, TextureWrapMode::MIRRORED_REPEAT, TextureWrapMode::MIRRORED_REPEAT, TextureFilterMode::NEAREST_MIPMAP_LINEAR, TextureFilterMode::LINEAR, flip, false);
}

Texture::Texture(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, flip, false);
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

glm::ivec2 Texture::GetSize() const {
	return _size;
}

int Texture::GetWidth() const
{
	return _size.x;
}

int Texture::GetHeight() const
{
	return _size.y;
}

int Texture::GetChannelsNum() const
{
	return _channelsNum;
}

TextureFormat Texture::GetFormat() const
{
	return _format;
}

TextureFileFormat Texture::GetFileFormat() const
{
	return _fileFormat;
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