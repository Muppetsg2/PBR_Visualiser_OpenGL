#pragma once

class Shader
{
private:
    GLuint _programId;
    const GLchar* _vertPath;
    const GLchar* _fragPath;
    bool _init = false;
public:

    Shader();
    Shader(const Shader&& shader);
    Shader(const Shader& shader);
    Shader(const GLchar* vertPath, const GLchar* fragPath);
    Shader(GLuint programId, const GLchar* vertPath, const GLchar* fragPath);

    virtual ~Shader();

    GLuint GetProgramId() const;
    bool IsInitialized() const;

    void Use() const;
    void Relode();

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetUInt(const std::string& name, unsigned int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetBVec2(const std::string& name, const glm::bvec2& value) const;
    void SetBVec3(const std::string& name, const glm::bvec3& value) const;
    void SetBVec4(const std::string& name, const glm::bvec4& value) const;
    void SetIVec2(const std::string& name, const glm::ivec2& value) const;
    void SetIVec3(const std::string& name, const glm::ivec3& value) const;
    void SetIVec4(const std::string& name, const glm::ivec4& value) const;
    void SetUVec2(const std::string& name, const glm::uvec2& value) const;
    void SetUVec3(const std::string& name, const glm::uvec3& value) const;
    void SetUVec4(const std::string& name, const glm::uvec4& value) const;
    void SetDVec2(const std::string& name, const glm::dvec2& value) const;
    void SetDVec3(const std::string& name, const glm::dvec3& value) const;
    void SetDVec4(const std::string& name, const glm::dvec4& value) const;
    void SetMat3(const std::string& name, glm::mat3& value) const;
    void SetMat4(const std::string& name, glm::mat4& value) const;

    bool GetBool(const std::string& name) const;
    int GetInt(const std::string& name) const;
    unsigned int GetUInt(const std::string& name) const;
    float GetFloat(const std::string& name) const;
    glm::vec2 GetVec2(const std::string& name) const;
    glm::vec3 GetVec3(const std::string& name) const;
    glm::vec4 GetVec4(const std::string& name) const;
    glm::bvec2 GetBVec2(const std::string& name) const;
    glm::bvec3 GetBVec3(const std::string& name) const;
    glm::bvec4 GetBVec4(const std::string& name) const;
    glm::ivec2 GetIVec2(const std::string& name) const;
    glm::ivec3 GetIVec3(const std::string& name) const;
    glm::ivec4 GetIVec4(const std::string& name) const;
    glm::uvec2 GetUVec2(const std::string& name) const;
    glm::uvec3 GetUVec3(const std::string& name) const;
    glm::uvec4 GetUVec4(const std::string& name) const;
    glm::dvec2 GetDVec2(const std::string& name) const;
    glm::dvec3 GetDVec3(const std::string& name) const;
    glm::dvec4 GetDVec4(const std::string& name) const;
    glm::mat3 GetMat3(const std::string& name) const;
    glm::mat4 GetMat4(const std::string& name) const;
};