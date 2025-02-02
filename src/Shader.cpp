#include <Shader.h>
#include <ShadersExtractor.h>

void Shader::LoadShaderFromFile(const GLchar* vertPath, const GLchar* fragPath) 
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertPath);
        fShaderFile.open(fragPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        spdlog::error("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
        return;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader  
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(vertex);
        return;
    };

    // Fragment Shader  
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(fragment);
        return;
    };

    // Program Object  
    _programId = glCreateProgram();
    glAttachShader(_programId, vertex);
    glAttachShader(_programId, fragment);
    glLinkProgram(_programId);

    glGetProgramiv(_programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", infoLog);
        glDeleteProgram(_programId);
        _programId = 0;
        return;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    _init = true;

    _vertPath = vertPath;
    _fragPath = fragPath;
}

void Shader::LoadShaderFromFile(const GLchar* vertPath, const GLchar* geomPath, const GLchar* fragPath)
{
    std::string vertexCode;
    std::string geometryCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream gShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vShaderFile.open(vertPath);
        gShaderFile.open(geomPath);
        fShaderFile.open(fragPath);
        std::stringstream vShaderStream, gShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        gShaderStream << gShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        gShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        geometryCode = gShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        spdlog::error("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
        return;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, geometry, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader  
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(vertex);
        return;
    };

    // Geometry Shader  
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gShaderCode, NULL);
    glCompileShader(geometry);

    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(geometry, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(geometry);
        return;
    };

    // Fragment Shader  
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(fragment);
        return;
    };

    // Program Object  
    _programId = glCreateProgram();
    glAttachShader(_programId, vertex);
    glAttachShader(_programId, geometry);
    glAttachShader(_programId, fragment);
    glLinkProgram(_programId);

    glGetProgramiv(_programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", infoLog);
        glDeleteProgram(_programId);
        _programId = 0;
        return;
    }

    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);

    _init = true;

    _vertPath = vertPath;
    _geomPath = geomPath;
    _fragPath = fragPath;

    _hasGeom = true;
}

void Shader::LoadShaderFromExtractor(std::string vertName, std::string fragName)
{
    std::string vertexCode = ShadersExtractor::GetShaderContent(vertName);
    std::string fragmentCode = ShadersExtractor::GetShaderContent(fragName);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader  
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(vertex);
        return;
    };

    // Fragment Shader  
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(fragment);
        return;
    };

    // Program Object  
    _programId = glCreateProgram();
    glAttachShader(_programId, vertex);
    glAttachShader(_programId, fragment);
    glLinkProgram(_programId);

    glGetProgramiv(_programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", infoLog);
        glDeleteProgram(_programId);
        _programId = 0;
        return;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    _init = true;

    _vertPath = strdup(vertName.c_str());
    _fragPath = strdup(fragName.c_str());

    _fromExtractor = true;
}

void Shader::LoadShaderFromExtractor(std::string vertName, std::string geomName, std::string fragName)
{
    std::string vertexCode = ShadersExtractor::GetShaderContent(vertName);
    std::string geometryCode = ShadersExtractor::GetShaderContent(geomName);
    std::string fragmentCode = ShadersExtractor::GetShaderContent(fragName);

    const char* vShaderCode = vertexCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, geometry, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader  
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(vertex);
        return;
    };

    // Geometry Shader  
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gShaderCode, NULL);
    glCompileShader(geometry);

    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(geometry, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(geometry);
        return;
    };


    // Fragment Shader  
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}", infoLog);
        glDeleteShader(fragment);
        return;
    };

    // Program Object  
    _programId = glCreateProgram();
    glAttachShader(_programId, vertex);
    glAttachShader(_programId, geometry);
    glAttachShader(_programId, fragment);
    glLinkProgram(_programId);

    glGetProgramiv(_programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", infoLog);
        glDeleteProgram(_programId);
        _programId = 0;
        return;
    }

    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);

    _init = true;

    _vertPath = strdup(vertName.c_str());
    _geomPath = strdup(vertName.c_str());
    _fragPath = strdup(fragName.c_str());

    _fromExtractor = true;
    _hasGeom = true;
}

Shader::Shader()
{
    _programId = 0;
    _vertPath = "";
    _geomPath = "";
    _fragPath = "";
    _hasGeom = false;
    _fromExtractor = false;
    _init = false;
}

Shader::Shader(const Shader&& shader) noexcept
{
    _programId = shader._programId;
    _vertPath = shader._vertPath;
    _geomPath = shader._geomPath;
    _fragPath = shader._fragPath;
    _hasGeom = shader._hasGeom;
    _fromExtractor = shader._fromExtractor;
    _init = shader._init;
}

Shader::Shader(const Shader& shader)
{
    _programId = shader._programId;
    _vertPath = shader._vertPath;
    _geomPath = shader._geomPath;
    _fragPath = shader._fragPath;
    _hasGeom = shader._hasGeom;
    _fromExtractor = shader._fromExtractor;
    _init = shader._init;
}

Shader::Shader(const GLchar* vertPath, const GLchar* fragPath)
{
    LoadShaderFromFile(vertPath, fragPath);
}

Shader::Shader(const GLchar* vertPath, const GLchar* geomPath, const GLchar* fragPath)
{
    LoadShaderFromFile(vertPath, geomPath, fragPath);
}

Shader::Shader(GLuint programId, const GLchar* vertPath, const GLchar* fragPath)
{
    _programId = programId;
    _vertPath = vertPath;
    _fragPath = fragPath;
    _geomPath = "";
    _hasGeom = false;
    _fromExtractor = false;
    _init = true;
}

Shader::Shader(GLuint programId, const GLchar* vertPath, const GLchar* geomPath, const GLchar* fragPath)
{
    _programId = programId;
    _vertPath = vertPath;
    _geomPath = geomPath;
    _fragPath = fragPath;
    _hasGeom = true;
    _fromExtractor = false;
    _init = true;
}

Shader* Shader::FromFile(const GLchar* vertPath, const GLchar* fragPath)
{
    Shader* res = new Shader();

    res->LoadShaderFromFile(vertPath, fragPath);

    if (!res->IsInitialized()) {
        spdlog::error("ERROR::SHADER::CREATE::CREATION_FROM_FILE_FAILED");
        delete res;
        return nullptr;
    }

    return res;
}

Shader* Shader::FromFile(const GLchar* vertPath, const GLchar* geomPath, const GLchar* fragPath)
{
    Shader* res = new Shader();

    res->LoadShaderFromFile(vertPath, geomPath, fragPath);

    if (!res->IsInitialized()) {
        spdlog::error("ERROR::SHADER::CREATE::CREATION_FROM_FILE_FAILED");
        delete res;
        return nullptr;
    }

    return res;
}

Shader* Shader::FromExtractor(std::string vertName, std::string fragName)
{
    Shader* res = new Shader();

    res->LoadShaderFromExtractor(vertName, fragName);

    if (!res->IsInitialized()) {
        spdlog::error("ERROR::SHADER::CREATE::CREATION_FROM_EXTRACTOR_FAILED");
        delete res;
        return nullptr;
    }

    return res;
}

Shader* Shader::FromExtractor(std::string vertName, std::string geomName, std::string fragName)
{
    Shader* res = new Shader();

    res->LoadShaderFromExtractor(vertName, geomName, fragName);

    if (!res->IsInitialized()) {
        spdlog::error("ERROR::SHADER::CREATE::CREATION_FROM_EXTRACTOR_FAILED");
        delete res;
        return nullptr;
    }

    return res;
}

Shader::~Shader()
{
    glDeleteProgram(_programId);
}

void Shader::Use() const
{
    glUseProgram(_programId);
}

void Shader::Relode()
{
    unsigned int temp = _programId;
    _init = false;

    if (_fromExtractor) {
        if (_hasGeom) {
            LoadShaderFromExtractor(_vertPath, _geomPath, _fragPath);
        }
        else {
            LoadShaderFromExtractor(_vertPath, _fragPath);
        }
    }
    else {
        if (_hasGeom) {
            LoadShaderFromFile(_vertPath, _geomPath, _fragPath);
        }
        else {
            LoadShaderFromFile(_vertPath, _fragPath);
        }
    }

    if (_init) {
        glDeleteProgram(temp);
    }
    else {
        spdlog::error("ERROR::SHADER::RELOAD::RELOADING_FAILED");
        _programId = temp;
        _init = true;
    }
}

GLuint Shader::GetProgramId() const
{
    return _programId;
}

bool Shader::IsInitialized() const
{
    return _init;
}
 
void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(_programId, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(_programId, name.c_str()), value);
}

void Shader::SetUInt(const std::string& name, unsigned int value) const
{
    glUniform1ui(glGetUniformLocation(_programId, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(_programId, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetBVec2(const std::string& name, const glm::bvec2& value) const
{
    glUniform2iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr((glm::ivec2)value));
}

void Shader::SetBVec3(const std::string& name, const glm::bvec3& value) const
{
    glUniform3iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr((glm::ivec3)value));
}

void Shader::SetBVec4(const std::string& name, const glm::bvec4& value) const
{
    glUniform4iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr((glm::ivec4)value));
}

void Shader::SetIVec2(const std::string& name, const glm::ivec2& value) const
{
    glUniform2iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetIVec3(const std::string& name, const glm::ivec3& value) const
{
    glUniform3iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetIVec4(const std::string& name, const glm::ivec4& value) const
{
    glUniform4iv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetUVec2(const std::string& name, const glm::uvec2& value) const
{
    glUniform2uiv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetUVec3(const std::string& name, const glm::uvec3& value) const
{
    glUniform3uiv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetUVec4(const std::string& name, const glm::uvec4& value) const
{
    glUniform4uiv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetDVec2(const std::string& name, const glm::dvec2& value) const
{
    glUniform2dv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetDVec3(const std::string& name, const glm::dvec3& value) const
{
    glUniform3dv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetDVec4(const std::string& name, const glm::dvec4& value) const
{
    glUniform4dv(glGetUniformLocation(_programId, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

bool Shader::GetBool(const std::string& name) const
{
    GLint ret[1]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 1 * sizeof(GLint), ret);

    return (bool)ret[0];
}

int Shader::GetInt(const std::string& name) const
{
    GLint ret[1]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 1 * sizeof(GLint), ret);

    return ret[0];
}

unsigned int Shader::GetUInt(const std::string& name) const
{
    GLuint ret[1]{};

    glGetnUniformuiv(_programId, glGetUniformLocation(_programId, name.c_str()), 1 * sizeof(GLuint), ret);

    return ret[0];
}

float Shader::GetFloat(const std::string& name) const
{
    GLfloat ret[1]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 1 * sizeof(GLfloat), ret);

    return ret[0];
}

glm::vec2 Shader::GetVec2(const std::string& name) const
{
    GLfloat ret[2]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 2 * sizeof(GLfloat), ret);

    return glm::make_vec2(ret);
}

glm::vec3 Shader::GetVec3(const std::string& name) const
{
    GLfloat ret[3]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 3 * sizeof(GLfloat), ret);

    return glm::make_vec3(ret);
}

glm::vec4 Shader::GetVec4(const std::string& name) const
{
    GLfloat ret[4]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 4 * sizeof(GLfloat), ret);

    return glm::make_vec4(ret);
}

glm::bvec2 Shader::GetBVec2(const std::string& name) const
{
    GLint ret[2]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 2 * sizeof(GLint), ret);

    return (glm::bvec2)glm::make_vec2(ret);
}

glm::bvec3 Shader::GetBVec3(const std::string& name) const
{
    GLint ret[3]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 3 * sizeof(GLint), ret);

    return (glm::bvec3)glm::make_vec3(ret);
}

glm::bvec4 Shader::GetBVec4(const std::string& name) const
{
    GLint ret[4]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 4 * sizeof(GLint), ret);

    return (glm::bvec4)glm::make_vec4(ret);
}

glm::ivec2 Shader::GetIVec2(const std::string& name) const
{
    GLint ret[2]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 2 * sizeof(GLint), ret);

    return glm::make_vec2(ret);
}

glm::ivec3 Shader::GetIVec3(const std::string& name) const
{
    GLint ret[3]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 3 * sizeof(GLint), ret);

    return glm::make_vec3(ret);
}

glm::ivec4 Shader::GetIVec4(const std::string& name) const
{
    GLint ret[4]{};

    glGetnUniformiv(_programId, glGetUniformLocation(_programId, name.c_str()), 4 * sizeof(GLint), ret);

    return glm::make_vec4(ret);
}

glm::uvec2 Shader::GetUVec2(const std::string& name) const
{
    GLuint ret[2]{};

    glGetnUniformuiv(_programId, glGetUniformLocation(_programId, name.c_str()), 2 * sizeof(GLuint), ret);

    return glm::make_vec2(ret);
}

glm::uvec3 Shader::GetUVec3(const std::string& name) const
{
    GLuint ret[3]{};

    glGetnUniformuiv(_programId, glGetUniformLocation(_programId, name.c_str()), 3 * sizeof(GLuint), ret);

    return glm::make_vec3(ret);
}

glm::uvec4 Shader::GetUVec4(const std::string& name) const
{
    GLuint ret[4]{};

    glGetnUniformuiv(_programId, glGetUniformLocation(_programId, name.c_str()), 4 * sizeof(GLuint), ret);

    return glm::make_vec4(ret);
}

glm::dvec2 Shader::GetDVec2(const std::string& name) const
{
    GLdouble ret[2]{};

    glGetnUniformdv(_programId, glGetUniformLocation(_programId, name.c_str()), 2 * sizeof(GLdouble), ret);

    return glm::make_vec2(ret);
}

glm::dvec3 Shader::GetDVec3(const std::string& name) const
{
    GLdouble ret[3]{};

    glGetnUniformdv(_programId, glGetUniformLocation(_programId, name.c_str()), 3 * sizeof(GLdouble), ret);

    return glm::make_vec3(ret);
}

glm::dvec4 Shader::GetDVec4(const std::string& name) const
{
    GLdouble ret[4]{};

    glGetnUniformdv(_programId, glGetUniformLocation(_programId, name.c_str()), 4 * sizeof(GLdouble), ret);

    return glm::make_vec4(ret);
}

glm::mat3 Shader::GetMat3(const std::string& name) const
{
    GLfloat tab[9]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 9 * sizeof(GLfloat), tab);

    return glm::make_mat3(tab);
}

glm::mat4 Shader::GetMat4(const std::string& name) const
{
    GLfloat tab[16]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 16 * sizeof(GLfloat), tab);

    return glm::make_mat4(tab);
}