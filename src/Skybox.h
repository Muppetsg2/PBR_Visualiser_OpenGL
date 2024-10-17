#pragma once

#include <Shader.h>

class Skybox {
private:

    static GLuint _vao;
    static GLuint _texture;
    static Shader* _shader;

    static GLuint _irradianceTexture;

    static bool _init;
    static bool _hdr;

    static const GLchar* _paths[6];

    static GLFWwindow* _window;

    Skybox() = default;
    virtual ~Skybox() = default;

public:
    static void Init(GLFWwindow* window, const GLchar* faces[6]);
    static void Init(GLFWwindow* window, const GLchar* hdr);
    static void Draw();
    static void Deinit();

    static void UseTexture(unsigned int samplerId);
    static void UseIrradianceTexture(unsigned int samplerId);
};