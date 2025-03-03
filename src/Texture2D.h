#pragma once
#include <macros.h>

#undef RGB

ENUM_CLASS_VALUE(TextureFormat,
	RED, GL_RED,
	RG, GL_RG,
	RGB, GL_RGB,
	BGR, GL_BGR,
	RGBA, GL_RGBA,
	BGRA, GL_BGRA,
	R_INT, GL_RED_INTEGER,
	RG_INT, GL_RG_INTEGER,
	RGB_INT, GL_RGB_INTEGER,
	BGR_INT, GL_BGR_INTEGER,
	RGBA_INT, GL_RGBA_INTEGER,
	BGRA_INT, GL_BGRA_INTEGER,
	STENCIL_IDX, GL_STENCIL_INDEX,
	DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
	DEPTH_STENCIL, GL_DEPTH_STENCIL
)

ENUM_CLASS_VALUE(TextureFileFormat,
	DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
	DEPTH_STENCIL, GL_DEPTH_STENCIL,
	RED, GL_RED,
	RG, GL_RG,
	RGB, GL_RGB,
	SRGB, GL_SRGB,
	RGBA, GL_RGBA,
	SRGBA, GL_SRGB_ALPHA,
	R8, GL_R8,
	R8_SNORM, GL_R8_SNORM,
	R16, GL_R16,
	R16_SNORM, GL_R16_SNORM,
	RG8, GL_RG8,
	RG8_SNORM, GL_RG8_SNORM,
	RG16, GL_RG16,
	RG16_SNORM, GL_RG16_SNORM,
	RG3_B2, GL_R3_G3_B2,
	RGB4, GL_RGB4,
	RGB5, GL_RGB5,
	RGB8, GL_RGB8,
	RGB8_SNORM, GL_RGB8_SNORM,
	SRGB8, GL_SRGB8,
	RGB10, GL_RGB10,
	RGB12, GL_RGB12,
	RGB16_SNORM, GL_RGB16_SNORM,
	RGBA2, GL_RGBA2,
	RGBA4, GL_RGBA4,
	RGB5_A1, GL_RGB5_A1,
	RGBA8, GL_RGBA8,
	RGBA8_SNORM, GL_RGBA8_SNORM,
	SRGBA8, GL_SRGB8_ALPHA8,
	RGB10_A2, GL_RGB10_A2,
	RGB10_A2_UINT, GL_RGB10_A2UI,
	RGBA12, GL_RGBA12,
	RGBA16, GL_RGBA16,
	R16_FLOAT, GL_R16F,
	RG16_FLOAT, GL_RG16F,
	RGB16_FLOAT, GL_RGB16F,
	RGBA16_FLOAT, GL_RGBA16F,
	R32_FLOAT, GL_R32F,
	RG32_FLOAT, GL_RG32F,
	RGB32_FLOAT, GL_RGB32F,
	RGBA32_FLOAT, GL_RGBA32F,
	RG11_B10_FLOAT, GL_R11F_G11F_B10F,
	RGB9_E5, GL_RGB9_E5,
	R8_INT, GL_R8I,
	R8_UINT, GL_R8UI,
	R16_INT, GL_R16I,
	R16_UINT, GL_R16UI,
	R32_INT, GL_R32I,
	R32_UINT, GL_R32UI,
	RG8_INT, GL_RG8I,
	RG8_UINT, GL_RG8UI,
	RG16_INT, GL_RG16I,
	RG16_UINT, GL_RG16UI,
	RG32_INT, GL_RG32I,
	RG32_UINT, GL_RG32UI,
	RGB8_INT, GL_RGB8I,
	RGB8_UINT, GL_RGB8UI,
	RGB16_INT, GL_RGB16I,
	RGB16_UINT, GL_RGB16UI,
	RGB32_INT, GL_RGB32I,
	RGB32_UINT, GL_RGB32UI,
	RGBA8_INT, GL_RGBA8I,
	RGBA8_UINT, GL_RGBA8UI,
	RGBA16_INT, GL_RGBA16I,
	RGBA16_UINT, GL_RGBA16UI,
	RGBA32_INT, GL_RGBA32I,
	RGBA32_UINT, GL_RGBA32UI,
	COMPRESSED_R, GL_COMPRESSED_RED,
	COMPRESSED_RG, GL_COMPRESSED_RG,
	COMPRESSED_RGB, GL_COMPRESSED_RGB,
	COMPRESSED_RGBA, GL_COMPRESSED_RGBA,
	COMPRESSED_SRGB, GL_COMPRESSED_SRGB,
	COMPRESSED_SRGBA, GL_COMPRESSED_SRGB_ALPHA,
	COMPRESSED_R_RGTC1, GL_COMPRESSED_RED_RGTC1,
	COMPRESSED_SIGNED_R_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1,
	COMPRESSED_RG_RGTC2, GL_COMPRESSED_RG_RGTC2,
	COMPRESSED_SIGNED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2,
	COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM,
	COMPRESSED_SRGBA_BPTC_UNORM, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
	COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
	COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
)

ENUM_CLASS_VALUE(TextureWrapMode,
	REPEAT, GL_REPEAT,
	CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER,
	CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
	MIRRORED_REPEAT, GL_MIRRORED_REPEAT,
	MIRROR_CLAMP_TO_EDGE, GL_MIRROR_CLAMP_TO_EDGE
)

ENUM_CLASS_VALUE(TextureFilterMode,
	NEAREST, GL_NEAREST,
	LINEAR, GL_LINEAR,
	NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR
)

class Texture2D {
private:
	GLuint _id;
	glm::ivec2 _size;
	int _channelsNum;
	bool _init = false;

	std::string _path;
	bool _hdr;

	TextureFormat _format;
	TextureFileFormat _fileFormat;
	TextureWrapMode _sWrapMode;
	TextureWrapMode _tWrapMode;
	TextureFilterMode _minFilterMode;
	TextureFilterMode _magFilterMode;
	
	int GetChannelsFromFileFormat(TextureFileFormat fileFormat);
	void GenerateGLTexture(const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool detectFormat);
	void LoadTextureFromFile(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip, bool detectFormat);

public:
	Texture2D();
	Texture2D(GLuint texId, glm::ivec2 size = glm::ivec2(1, 1), int channelsNum = 3, bool mipmap = false, bool hdr = false);
	Texture2D(const Texture2D&& texture) noexcept;
	Texture2D(const Texture2D& texture);
	Texture2D(const GLchar* path, bool flip = false);
	Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, bool flip = false);
	Texture2D(const GLchar* path, const TextureFileFormat& fileFormat, const TextureFormat& format, const TextureWrapMode& sWrapMode, const TextureWrapMode& tWrapMode, const TextureFilterMode& minFilterMode, const TextureFilterMode& magFilterMode, bool flip = false);
	virtual ~Texture2D();

	void SetWrapModeS(const TextureWrapMode& mode);
	void SetWrapModeT(const TextureWrapMode& mode);
	void SetMinFilterMode(const TextureFilterMode& mode);
	void SetMagFilterMode(const TextureFilterMode& mode);

	bool IsInit() const;

	GLuint GetId() const;
	glm::ivec2 GetSize() const;
	std::string GetPath() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetChannelsNum() const;
	TextureFormat GetFormat() const;
	TextureFileFormat GetFileFormat() const;
	TextureWrapMode GetWrapModeS() const;
	TextureWrapMode GetWrapModeT() const;
	TextureFilterMode GetMinFilterMode() const;
	TextureFilterMode GetMagFilterMode() const;

	void Use(GLuint samplerId = 0) const;
};