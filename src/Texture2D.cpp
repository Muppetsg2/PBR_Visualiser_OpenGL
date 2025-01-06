#include <Texture2D.h>

void Texture2D::GenerateGLTexture(const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool detectFormat)
{
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
		if (this->_channelsNum == 1) { this->_format = TextureFormat::RED; this->_fileFormat = this->_hdr ? TextureFileFormat::R32_FLOAT : TextureFileFormat::RED; }
		else if (this->_channelsNum == 2) { this->_format = TextureFormat::RG; this->_fileFormat = this->_hdr ? TextureFileFormat::RG32_FLOAT : TextureFileFormat::RG; }
		else if (this->_channelsNum == 3) { this->_format = TextureFormat::RGB; this->_fileFormat = this->_hdr ? TextureFileFormat::RGB32_FLOAT : TextureFileFormat::SRGB; }
		else if (this->_channelsNum == 4) { this->_format = TextureFormat::RGBA; this->_fileFormat = this->_hdr ? TextureFileFormat::RGBA32_FLOAT : TextureFileFormat::SRGBA; }
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::LoadTextureFromFile(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip, bool detectFormat)
{
	stbi_set_flip_vertically_on_load(flip);
	this->_hdr = (std::filesystem::path(std::string(path)).extension().string() == std::string(".hdr"));

	if (this->_hdr) {
		float* image = stbi_loadf(path, &(this->_size.x), &(this->_size.y), &(this->_channelsNum), 0);

		if (image) {
			this->_path = std::string(path);

			GenerateGLTexture(fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, detectFormat);

			glBindTexture(GL_TEXTURE_2D, this->_id);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)this->_fileFormat, this->_size.x, this->_size.y, 0, (GLenum)this->_format, GL_FLOAT, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
			spdlog::error("Failed to load texture: {}", path);
		}

		stbi_image_free(image);
	}
	else {
		unsigned char* image = stbi_load(path, &(this->_size.x), &(this->_size.y), &(this->_channelsNum), 0);

		if (image) {
			this->_path = std::string(path);

			GenerateGLTexture(fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, detectFormat);

			glBindTexture(GL_TEXTURE_2D, this->_id);
			glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)this->_fileFormat, this->_size.x, this->_size.y, 0, (GLenum)this->_format, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
			spdlog::error("Failed to load texture: {}", path);
		}

		stbi_image_free(image);
	}

	if (Config::isVerbose()) spdlog::info("Texture at path '{}' loaded!", path);
}

Texture2D::Texture2D()
{
	_id = 0;
	_size = { 0, 0 };
	_channelsNum = 0;

	_path = "";
	_hdr = false;

	_format = TextureFormat::RGB;
	_fileFormat = TextureFileFormat::SRGB;
	_sWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_tWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_minFilterMode = TextureFilterMode::NEAREST_MIPMAP_LINEAR;
	_magFilterMode = TextureFilterMode::LINEAR;
}

Texture2D::Texture2D(GLuint texId, glm::ivec2 size, int channelsNum, bool mipmap, bool hdr)
{
	_id = texId;
	_size = size;
	_channelsNum = channelsNum;

	_path = "";
	_hdr = hdr;

	switch (channelsNum) {
		case 1: {
			_format = TextureFormat::RED;
			_fileFormat = hdr ? TextureFileFormat::RED : TextureFileFormat::R32_FLOAT;
		}
		case 2: {
			_format = TextureFormat::RG;
			_fileFormat = hdr ? TextureFileFormat::RG : TextureFileFormat::RG32_FLOAT;
		}
		case 3: {
			_format = TextureFormat::RGB;
			_fileFormat = hdr ? TextureFileFormat::SRGB : TextureFileFormat::RGB32_FLOAT;
		}
		case 4: {
			_format = TextureFormat::RGBA;
			_fileFormat = hdr ? TextureFileFormat::SRGBA : TextureFileFormat::RGBA32_FLOAT;
		}
	}

	_sWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_tWrapMode = TextureWrapMode::MIRRORED_REPEAT;
	_minFilterMode = mipmap ? TextureFilterMode::NEAREST_MIPMAP_LINEAR : TextureFilterMode::LINEAR;
	_magFilterMode = TextureFilterMode::LINEAR;

	if (mipmap) {
		glBindTexture(GL_TEXTURE_2D, _id);
		glGenerateTextureMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

Texture2D::Texture2D(const Texture2D&& texture) noexcept
{
	_id = texture._id;
	_size = texture._size;
	_channelsNum = texture._channelsNum;

	_path = texture._path;
	_hdr = texture._hdr;

	_format = texture._format;
	_fileFormat = texture._fileFormat;
	_sWrapMode = texture._sWrapMode;
	_tWrapMode = texture._tWrapMode;
	_minFilterMode = texture._minFilterMode;
	_magFilterMode = texture._magFilterMode;
}

Texture2D::Texture2D(const Texture2D& texture)
{
	_id = texture._id;
	_size = texture._size;
	_channelsNum = texture._channelsNum;

	_path = texture._path;
	_hdr = texture._hdr;

	_format = texture._format;
	_fileFormat = texture._fileFormat;
	_sWrapMode = texture._sWrapMode;
	_tWrapMode = texture._tWrapMode;
	_minFilterMode = texture._minFilterMode;
	_magFilterMode = texture._magFilterMode;
}

Texture2D::Texture2D(const GLchar* path, bool flip)
{
	LoadTextureFromFile(path, TextureFileFormat::SRGB, TextureFormat::RGB, TextureWrapMode::MIRRORED_REPEAT, TextureWrapMode::MIRRORED_REPEAT, TextureFilterMode::NEAREST_MIPMAP_LINEAR, TextureFilterMode::LINEAR, flip, true);
}

Texture2D::Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, TextureWrapMode::MIRRORED_REPEAT, TextureWrapMode::MIRRORED_REPEAT, TextureFilterMode::NEAREST_MIPMAP_LINEAR, TextureFilterMode::LINEAR, flip, false);
}

Texture2D::Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, flip, false);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &_id);
}

void Texture2D::SetWrapModeS(const TextureWrapMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)mode);
	_sWrapMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetWrapModeT(const TextureWrapMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)mode);
	_tWrapMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetMinFilterMode(const TextureFilterMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
	_minFilterMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetMagFilterMode(const TextureFilterMode& mode)
{
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mode);
	_magFilterMode = mode;
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture2D::GetId() const
{
	return _id;
}

glm::ivec2 Texture2D::GetSize() const {
	return _size;
}

std::string Texture2D::GetPath() const
{
	return _path;
}

int Texture2D::GetWidth() const
{
	return _size.x;
}

int Texture2D::GetHeight() const
{
	return _size.y;
}

int Texture2D::GetChannelsNum() const
{
	return _channelsNum;
}

TextureFormat Texture2D::GetFormat() const
{
	return _format;
}

TextureFileFormat Texture2D::GetFileFormat() const
{
	return _fileFormat;
}

TextureWrapMode Texture2D::GetWrapModeS() const
{
	return _sWrapMode;
}

TextureWrapMode Texture2D::GetWrapModeT() const
{
	return _tWrapMode;
}

TextureFilterMode Texture2D::GetMinFilterMode() const
{
	return _minFilterMode;
}

TextureFilterMode Texture2D::GetMagFilterMode() const
{
	return _magFilterMode;
}

void Texture2D::Use(unsigned int samplerId) const
{
	glActiveTexture(GL_TEXTURE0 + samplerId);
	glBindTexture(GL_TEXTURE_2D, _id);
}