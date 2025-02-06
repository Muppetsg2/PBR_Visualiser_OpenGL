#pragma once

#if WINDOW_APP
#include <Shader.h>
#include <macros.h>

ENUM_CLASS_BASE_VALUE(SkyboxDisplay, uint8_t, DEFAULT, 0, IRRADIANCE, 1, PREFILTER, 2)
#endif

class Skybox {
private:
#if WINDOW_APP
    static Shader* _shader;
#endif
    static GLuint _vao;
    static GLuint _texture;

    static GLuint _irradianceTexture;
    static GLuint _prefilterTexture;
    static GLuint _brdfLUTTexture;

    static bool _init;

    static float _exposure;
    static float _colorIntensity;

#if WINDOW_APP
    static SkyboxDisplay _displayMode;
    static float _mipmapLevel;
    static float _MAX_MIPMAP_LEVEL_DEFAULT;
    static bool _hdr;
    static bool _fromData;
    static bool _openImageDialogs[8];
    static ImFileDialogInfo _imageDialogInfos[8];

    static std::string _paths[6];
#else
    static std::string _path;
#endif

    static GLFWwindow* _window;
    static glm::ivec2 _windowSize;

    Skybox() = delete;

    static bool GenerateBRDFLut(GLuint framebuffer, GLuint renderbuffer);

    static std::pair<bool, std::string> CheckFolder();
    static void SaveData(std::string dir);
    static bool LoadSavedData(std::string dir);
    static bool LoadSavedDataToChange(std::string dir);

public:
    static void Init(glm::ivec2 window_size, const GLchar* hdr);
    static void Init(GLFWwindow* window, const GLchar* hdr);
#if WINDOW_APP
    static void Init(glm::ivec2 window_size, const GLchar* faces[6]);
    static void Init(GLFWwindow* window, const GLchar* faces[6]);
    static void Draw();
#endif
    static void Deinit();

    static void UseTexture(unsigned int samplerId);
    static void UseIrradianceTexture(unsigned int samplerId);
    static void UsePrefilterTexture(unsigned int samplerId);
    static void UseBrdfLUTTexture(unsigned int samplerId);

    static float GetExposure();
    static float GetColorIntensity();

#if WINDOW_APP
    static SkyboxDisplay GetSkyboxDisplay();
    static float GetMipMapLevel();

    static void SetSkyboxDisplay(SkyboxDisplay mode);
    static void SetMipMapLevel(float value);
#endif

    static void SetExposure(float value);
    static void SetColorIntensity(float value);

    static void ChangeTexture(const GLchar* hdr);
#if WINDOW_APP
    static void ChangeTexture(const GLchar* faces[6]);
    static void DrawEditor(bool* open);
    static void DrawSkyboxFacesLoader(bool* open);
#endif
};