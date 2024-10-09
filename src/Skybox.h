#pragma once

#include <Shader.h>

class Skybox {
private:

    static GLuint _vao;
    static GLuint _texture;
    static Shader* _shader;

    static bool _init;
    static bool _hdri;

    static const GLchar* paths[6];

    Skybox() = default;
    virtual ~Skybox() = default;

public:
    static void Init(const GLchar* faces[6]);
    static void Init(const GLchar* hdri);
    static void Draw();
    static void Deinit();
};