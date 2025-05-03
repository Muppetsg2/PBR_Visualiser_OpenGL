#include <Texture2D.h>

int Texture2D::GetChannelsFromFileFormat(TextureFileFormat fileFormat) {
	switch (fileFormat) {
		// 1 channel
		case TextureFileFormat::R32_FLOAT:
		case TextureFileFormat::R8_SNORM:
		case TextureFileFormat::COMPRESSED_R:
		case TextureFileFormat::DEPTH_COMPONENT:
		case TextureFileFormat::R8:
		case TextureFileFormat::R16:
		case TextureFileFormat::R16_SNORM:
		case TextureFileFormat::R8_INT:
		case TextureFileFormat::R8_UINT:
		case TextureFileFormat::R16_INT:
		case TextureFileFormat::R16_UINT:
		case TextureFileFormat::R32_INT:
		case TextureFileFormat::R32_UINT:
		case TextureFileFormat::COMPRESSED_R_RGTC1:
		case TextureFileFormat::COMPRESSED_SIGNED_R_RGTC1:
			return 1;

		// 2 channels
		case TextureFileFormat::RG:
		case TextureFileFormat::RG8:
		case TextureFileFormat::RG8_SNORM:
		case TextureFileFormat::RG16:
		case TextureFileFormat::RG16_SNORM:
		case TextureFileFormat::RG3_B2:
		case TextureFileFormat::RG8_INT:
		case TextureFileFormat::RG8_UINT:
		case TextureFileFormat::RG16_INT:
		case TextureFileFormat::RG16_UINT:
		case TextureFileFormat::RG32_INT:
		case TextureFileFormat::RG32_UINT:
		case TextureFileFormat::COMPRESSED_RG:
		case TextureFileFormat::COMPRESSED_RG_RGTC2:
		case TextureFileFormat::COMPRESSED_SIGNED_RG_RGTC2:
			return 2;

		// 3 channels
		case TextureFileFormat::RGB:
		case TextureFileFormat::SRGB:
		case TextureFileFormat::RGB4:
		case TextureFileFormat::RGB5:
		case TextureFileFormat::RGB8:
		case TextureFileFormat::RGB8_SNORM:
		case TextureFileFormat::SRGB8:
		case TextureFileFormat::RGB10:
		case TextureFileFormat::RGB12:
		case TextureFileFormat::RGB16_SNORM:
		case TextureFileFormat::RGB8_INT:
		case TextureFileFormat::RGB8_UINT:
		case TextureFileFormat::RGB16_INT:
		case TextureFileFormat::RGB16_UINT:
		case TextureFileFormat::RGB32_INT:
		case TextureFileFormat::RGB32_UINT:
		case TextureFileFormat::RGB16_FLOAT:
		case TextureFileFormat::RGB32_FLOAT:
		case TextureFileFormat::RGB9_E5:
		case TextureFileFormat::RGB10_A2:
		case TextureFileFormat::RGB10_A2_UINT:
		case TextureFileFormat::COMPRESSED_RGB:
			return 3;

		// 4 channels
		case TextureFileFormat::RGBA:
		case TextureFileFormat::RGBA2:
		case TextureFileFormat::RGBA4:
		case TextureFileFormat::RGB5_A1:
		case TextureFileFormat::RGBA8:
		case TextureFileFormat::RGBA8_SNORM:
		case TextureFileFormat::RGBA12:
		case TextureFileFormat::RGBA16:
		case TextureFileFormat::RGBA8_INT:
		case TextureFileFormat::RGBA8_UINT:
		case TextureFileFormat::RGBA16_INT:
		case TextureFileFormat::RGBA16_UINT:
		case TextureFileFormat::RGBA32_INT:
		case TextureFileFormat::RGBA32_UINT:
		case TextureFileFormat::RGBA16_FLOAT:
		case TextureFileFormat::RGBA32_FLOAT:
		case TextureFileFormat::COMPRESSED_RGBA:
		case TextureFileFormat::COMPRESSED_SRGB:
		case TextureFileFormat::COMPRESSED_SRGBA:
		case TextureFileFormat::COMPRESSED_RGBA_BPTC_UNORM:
		case TextureFileFormat::COMPRESSED_SRGBA_BPTC_UNORM:
		case TextureFileFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case TextureFileFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			return 4;

		default:
			return 0;
	}
}

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
	this->_hdr = (std::filesystem::path(path).extension() == ".hdr");
	bool err = false;

	int actualChannels = 0;
	void* image = this->_hdr ? (void*)stbi_loadf(path, &(this->_size.x), &(this->_size.y), &actualChannels, 0)
							 : (void*)stbi_load(path, &(this->_size.x), &(this->_size.y), &actualChannels, 0);
	if (image) {
		int predChannels = GetChannelsFromFileFormat(fileFormat);

		if (actualChannels < predChannels) {
			spdlog::error("Texture '{}' has fewer channels ({}) than required by the format ({}).", path, actualChannels, predChannels);
			stbi_image_free(image);
			return;
		}

		this->_path = path;
		this->_channelsNum = actualChannels;
		GenerateGLTexture(fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, detectFormat);

		glBindTexture(GL_TEXTURE_2D, this->_id);
		glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)this->_fileFormat, this->_size.x, this->_size.y, 0, (GLenum)this->_format, this->_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		spdlog::error("Failed to load texture: {}", path);
		err = true;
	}

	stbi_image_free(image);

	_init = !err;

	if (Config::IsVerbose() && !err) spdlog::info("Texture at path '{}' loaded!", path);
}

Texture2D::Texture2D()
{
	_id = 0;
	_size = { 0, 0 };
	_channelsNum = 0;
	_init = false;

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
			break;
		}
		case 2: {
			_format = TextureFormat::RG;
			_fileFormat = hdr ? TextureFileFormat::RG : TextureFileFormat::RG32_FLOAT;
			break;
		}
		case 3: {
			_format = TextureFormat::RGB;
			_fileFormat = hdr ? TextureFileFormat::SRGB : TextureFileFormat::RGB32_FLOAT;
			break;
		}
		case 4: {
			_format = TextureFormat::RGBA;
			_fileFormat = hdr ? TextureFileFormat::SRGBA : TextureFileFormat::RGBA32_FLOAT;
			break;
		}
		default: {
			_format = TextureFormat::RGB;
			_fileFormat = hdr ? TextureFileFormat::SRGB : TextureFileFormat::RGB32_FLOAT;
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

	_init = true;
}

Texture2D::Texture2D(const Texture2D&& texture) noexcept
{
	_id = texture._id;
	_size = texture._size;
	_channelsNum = texture._channelsNum;
	_init = texture._init;

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
	_init = texture._init;

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

	if (_init == false) _hdr = false;
}

Texture2D::Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, TextureWrapMode::MIRRORED_REPEAT, TextureWrapMode::MIRRORED_REPEAT, TextureFilterMode::NEAREST_MIPMAP_LINEAR, TextureFilterMode::LINEAR, flip, false);

	if (_init == false) _hdr = false;
}

Texture2D::Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip)
{
	LoadTextureFromFile(path, fileFormat, format, sWrapMode, tWrapMode, minFilterMode, magFilterMode, flip, false);

	if (_init == false) _hdr = false;
}

Texture2D::~Texture2D()
{
	if (_init) glDeleteTextures(1, &_id);
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

bool Texture2D::IsInit() const
{
	return _init;
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