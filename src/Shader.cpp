#include <Shader.h>

Shader::Shader(const Shader&& shader)
{
    _programId = shader._programId;
    _vertPath = shader._vertPath;
    _fragPath = shader._fragPath;
    _init = shader._init;
}

Shader::Shader(const Shader& shader)
{
    _programId = shader._programId;
    _vertPath = shader._vertPath;
    _fragPath = shader._fragPath;
    _init = shader._init;
}

Shader::Shader(const GLchar* vertPath, const GLchar* fragPath)
{
    // 1. pobierz kod �r�d�owy Vertex/Fragment Shadera z filePath  
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    bool err = false;

    // zapewnij by obiekt ifstream m�g� rzuca� wyj�tkami  
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // otw�rz pliki  
        vShaderFile.open(vertPath);
        fShaderFile.open(fragPath);
        std::stringstream vShaderStream, fShaderStream;
        // zapisz zawarto�� bufora pliku do strumieni  
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // zamknij uchtywy do plik�w  
        vShaderFile.close();
        fShaderFile.close();
        // zamie� strumie� w �a�cuch znak�w  
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        spdlog::error("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
        err = true;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. skompiluj shadery  
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader  
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // wypisz b��dy kompilacji, je�li s� jakie�  
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
        err = true;
    };

    // Fragment Shader  
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // wypisz b��dy kompilacji, je�li s� jakie�  
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
        err = true;
    };

    // Program Object  
    _programId = glCreateProgram();
    glAttachShader(_programId, vertex);
    glAttachShader(_programId, fragment);
    glLinkProgram(_programId);
    // wypisz b��dy linkowania, je�li s� jakie�
    glGetProgramiv(_programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
        err = true;
    }

    // usu� obiekty shader'�w, kt�re s� ju� powi�zane  
    // z Program Object - nie b�d� nam ju� potrzebne  
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!err) {
        _init = true;

        _vertPath = vertPath;
        _fragPath = fragPath;
    }
}

Shader::Shader(GLuint programId, const GLchar* vertPath, const GLchar* fragPath)
{
    _programId = programId;
    _vertPath = vertPath;
    _fragPath = fragPath;
    _init = true;
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
    Shader* temp = new Shader(_vertPath, _fragPath);

    if (temp != nullptr) {
        if (temp->IsInitialized()) {
            GLuint i = this->_programId;
            this->_programId = temp->_programId;
            temp->_programId = i;
        }
    }

    delete temp;
}

GLuint Shader::GetProgramId() const
{
    return _programId;
}

bool Shader::IsInitialized() const
{
    return _init;
}

// funkcje operuj�ce na uniformach  
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

void Shader::SetMat4(const std::string& name, glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

glm::mat4 Shader::GetMat4(const std::string& name) const
{
    GLfloat tab[16]{};

    glGetnUniformfv(_programId, glGetUniformLocation(_programId, name.c_str()), 16 * sizeof(GLfloat), tab);

    glm::mat4 mat = glm::make_mat4(tab);

    return mat;
}