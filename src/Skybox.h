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

    static const GLchar* _paths[6];

    static GLFWwindow* _window;

    Skybox() = default;
    virtual ~Skybox() = default;

    static bool GenerateBRDFLut(GLuint framebuffer, GLuint renderbuffer);

    static std::pair<bool, std::string> CheckFolder();
    static void SaveData(std::string dir);
    static bool LoadSavedData(std::string dir);

public:
    static void Init(GLFWwindow* window, const GLchar* faces[6]);
    static void Init(GLFWwindow* window, const GLchar* hdr);
    static void Draw();
    static void Deinit();

    static void UseTexture(unsigned int samplerId);
    static void UseIrradianceTexture(unsigned int samplerId);
    static void UsePrefilterTexture(unsigned int samplerId);
    static void UseBrdfLUTTexture(unsigned int samplerId);
};