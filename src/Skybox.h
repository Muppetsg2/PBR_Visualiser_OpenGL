#pragma once

#include <Shader.h>

class Skybox {
private:

    static GLuint _vao;
    static GLuint _texture;
    static Shader* _shader;

    static GLuint _irradianceTexture;
    static GLuint _prefilterTexture;
    static GLuint _brdfLUTTexture;

    static bool _init;
    static bool _hdr;

#if _DEBUG
    static bool _openImageDialogs[7];
    static ImFileDialogInfo _imageDialogInfos[7];
#endif

    static std::string _paths[6];

    static std::string _resDir;

    static GLFWwindow* _window;
    static glm::ivec2 _windowSize;

    Skybox() = default;
    virtual ~Skybox() = default;

    static bool GenerateBRDFLut(GLuint framebuffer, GLuint renderbuffer, std::string shaderDir);

    static std::pair<bool, std::string> CheckFolder();
    static void SaveData(std::string dir);
    static bool LoadSavedData(std::string dir);

#if _DEBUG
    static bool LoadSavedDataToChange(std::string dir, bool isDiffrent);
#endif

public:
    static void Init(glm::ivec2 window_size, std::string exeDir, const GLchar* faces[6]);
    static void Init(GLFWwindow* window, std::string exeDir, const GLchar* faces[6]);
    static void Init(glm::ivec2 window_size, std::string exeDir, const GLchar* hdr);
    static void Init(GLFWwindow* window, std::string exeDir, const GLchar* hdr);
    static void Draw();
    static void Deinit();

    static void UseTexture(unsigned int samplerId);
    static void UseIrradianceTexture(unsigned int samplerId);
    static void UsePrefilterTexture(unsigned int samplerId);
    static void UseBrdfLUTTexture(unsigned int samplerId);

#if _DEBUG
    static void ChangeTexture(const GLchar* hdr);
    static void ChangeTexture(std::string faces[6]);
    static void DrawEditor(bool* open);
    static void DrawSkyboxFacesLoader(bool* open);
#endif
};