#if !CONSOLE_ENABLED

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif

extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#include <TimeManager.h>
#include <Texture2D.h>
#include <Shader.h>
#include <Skybox.h>
#include <Camera.h>

#if _DEBUG
#include <Shape.h>
#endif

#if _DEBUG
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void GLAPIENTRY ErrorMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    //if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return; // Chce ignorowac notyfikacje

    std::string severityS = "";
    if (severity == GL_DEBUG_SEVERITY_HIGH) severityS = "HIGHT";
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) severityS = "MEDIUM";
    else if (severity == GL_DEBUG_SEVERITY_LOW) severityS = "LOW";
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) severityS = "NOTIFICATION";

    if (type == GL_DEBUG_TYPE_ERROR) {
        SPDLOG_ERROR("GL CALLBACK: type = ERROR, severity = {0}, message = {1}\n", severityS, message);
    }
    else if (type == GL_DEBUG_TYPE_MARKER) {
        SPDLOG_INFO("GL CALLBACK: type = MARKER, severity = {0}, message = {1}\n", severityS, message);
    }
    else {
        std::string typeS = "";
        if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) typeS = "DEPRACTED BEHAVIOUR";
        else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) typeS = "UNDEFINED BEHAVIOUR";
        else if (type == GL_DEBUG_TYPE_PORTABILITY) typeS = "PORTABILITY";
        else if (type == GL_DEBUG_TYPE_PERFORMANCE) typeS = "PERFORMANCE";
        else if (type == GL_DEBUG_TYPE_PUSH_GROUP) typeS = "PUSH GROUP";
        else if (type == GL_DEBUG_TYPE_POP_GROUP) typeS = "POP GROUP";
        else if (type == GL_DEBUG_TYPE_OTHER) typeS = "OTHER";
        SPDLOG_WARN("GL CALLBACK: type = {0}, severity = {1}, message = {2}", typeS, severityS, message);
    }
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Camera::OnWindowSizeChange();
}
#endif

bool init();
void render();
GLuint LoadDefaultWhiteTexture();
GLuint LoadDefaultBlackTexture();

#if !_DEBUG
ENUM_CLASS_BASE_VALUE(RenderPosition, uint8_t, TOP, 0, BOTTOM, 1, FRONT, 2, BACK, 3, RIGHT, 4, LEFT, 5, DEFAULT, 6)

ENUM_CLASS_BASE_VALUE(RenderResolution, uint8_t, R128, 0, R256, 1, R512, 2, R1K, 3, R2K, 4, R4K, 5, DEFAULT, 6)

void printHelp();
void processFileArguments(int& i, int argc, char** argv, std::vector<std::string>& imgPaths);
void processNameArgument(const std::string& arg, std::string& fileName);
void processDirectoryArgument(const std::string& arg, std::string& direc);
void processPositionArgument(const std::string& arg, RenderPosition& position);
void processResolutionArgument(const std::string& arg, RenderResolution& resolution);
#else
void end_frame();
void input();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void init_imgui();
void imgui_begin();
void imgui_render();
void imgui_end();
#endif

constexpr const char* WINDOW_NAME = "PBR_Visualiser";
#if _DEBUG
constexpr int32_t WINDOW_WIDTH = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;
#else
int32_t WINDOW_WIDTH = 2048;
int32_t WINDOW_HEIGHT = 2048;
#endif

GLFWwindow* window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const     char*   glsl_version     = "#version 450";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 5;

Texture2D* imageTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
std::string imageName[6] = { "Albedo", "Normal", "Metallic", "Displacement", "Roughness", "AO" };
GLuint defaultWhiteTexture = 0;
GLuint defaultBlackTexture = 0;

GLuint quadVAO = 0;
Shader* PBR = nullptr;
glm::mat4 trans = glm::mat4(1.f);
float height_scale = 0.04f;

#if !_DEBUG
glm::vec3 quadVerts[4] = {
    { 0.f, -.5f, -.5f },
    {  0.f, -.5f, .5f  },
    { 0.f, .5f,  -.5f  },
    {  0.f, .5f,  .5f  }
};

unsigned int quadIndi[6] = {
    2, 1, 0,
    2, 3, 1
};
#endif

#if _DEBUG
bool openFileDialogs[6] = { false, false, false, false, false, false };
ImFileDialogInfo fileDialogInfos[6];

float cameraSpeed = 40.f;
bool released = true;
bool mouseNotUsed = true;
float sensitivity = 0.1f;

GLfloat lastX = 0.f, lastY = 0.f;
float rotateAngle = 50.f;
#endif

int main(int argc, char** argv)
{
#if _DEBUG
    spdlog::info("Configuration: DEBUG");
#else
    spdlog::info("Configuration: RELEASE");

    std::string fileName;
    std::string saveDir;
    std::vector<std::string> imgPaths;
    RenderPosition position = RenderPosition::DEFAULT;
    RenderResolution resolution = RenderResolution::DEFAULT;

    if (argc > 1) {
        bool expectName = false;
        bool expectDirectory = false;
        bool expectPosition = false;
        bool expectResolution = false;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "-h") {
                printHelp();
                imgPaths.clear();
                return 0;
            }

            if (arg == "-f") {
                ++i;
                processFileArguments(i, argc, argv, imgPaths);
            }
            else if (expectName) {
                processNameArgument(arg, fileName);
                expectName = false;
            }
            else if (expectDirectory) {
                processDirectoryArgument(arg, saveDir);
                expectDirectory = false;
            }
            else if (expectPosition) {
                processPositionArgument(arg, position);
                expectPosition = false;
            }
            else if (expectResolution) {
                processResolutionArgument(arg, resolution);
                expectResolution = false;
            }
            else if (arg == "-n") {
                expectName = true;
            }
            else if (arg == "-d") {
                expectDirectory = true;
            }
            else if (arg == "-p") {
                expectPosition = true;
            }
            else if (arg == "-r") {
                expectResolution = true;
            }
            else {
                spdlog::warn("Unknown argument: {}", arg);
            }
        }

        // Warnings if any expected argument is missing
        if (expectName) {
            spdlog::warn("The '-n' prefix was used, but no name was specified!");
        }
        if (expectDirectory) {
            spdlog::warn("The '-d' prefix was used, but no directory path was specified!");
        }
        if (expectPosition) {
            spdlog::warn("The '-p' prefix was used, but no position was specified!");
        }
        if (expectResolution) {
            spdlog::warn("The '-r' prefix was used, but no resolution was specified!");
        }

    }
    else {
        spdlog::info("No arguments were passed to the program.");
    }

    switch (resolution) {
        case RenderResolution::R128: {
            WINDOW_WIDTH = 128;
            WINDOW_HEIGHT = 128;
            break;
        }
        case RenderResolution::R256: {
            WINDOW_WIDTH = 256;
            WINDOW_HEIGHT = 256;
            break;
        }
        case RenderResolution::R512: {
            WINDOW_WIDTH = 512;
            WINDOW_HEIGHT = 512;
            break;
        }
        case RenderResolution::R1K: {
            WINDOW_WIDTH = 1024;
            WINDOW_HEIGHT = 1024;
            break;
        }
        case RenderResolution::R2K: {
            WINDOW_WIDTH = 2048;
            WINDOW_HEIGHT = 2048;
            break;
        }
        case RenderResolution::R4K: {
            WINDOW_WIDTH = 4096;
            WINDOW_HEIGHT = 4096;
            break;
        }
        case RenderResolution::DEFAULT: {
            WINDOW_WIDTH = 2048;
            WINDOW_HEIGHT = 2048;
            break;
        }
    }
#endif
    spdlog::info("Resolution: {}x{}", WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!init())
    {
        spdlog::error("Failed to initialize project!");
        return EXIT_FAILURE;
    }
    spdlog::info("Initialized project.");

#if _DEBUG
    init_imgui();
    spdlog::info("Initialized ImGui.");

    Camera::Init(window);
#else
    Camera::Init(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT));
#endif

#if _DEBUG
    Skybox::Init(window, "./res/skybox/rooitou_park_4k.hdr");
#else
    Skybox::Init(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT), "./res/skybox/rooitou_park_4k.hdr");

    for (size_t i = 0; i < imgPaths.size(); ++i) {
        TextureFileFormat inter = i == 0 ? TextureFileFormat::SRGB : i == 1 ? TextureFileFormat::RGB : TextureFileFormat::RED;
        TextureFormat form = i == 0 ? TextureFormat::RGB : i == 1 ? TextureFormat::RGB : TextureFormat::RED;
        imageTextures[i] = new Texture2D(imgPaths[i].c_str(), inter, form);
    }

    if (6 - imgPaths.size() > 0) {
        for (size_t z = imgPaths.size(); z < 6; ++z) {
            imageTextures[z] = new Texture2D(z >= 2 && z <= 4 ? defaultBlackTexture : defaultWhiteTexture);
        }
    }

    imgPaths.clear();
#endif

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

#if _DEBUG
    glBindBuffer(GL_ARRAY_BUFFER, Shape::GetSphereVBO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetSphereEBO());

    // Vertices positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(4);
#else
    GLuint quadVBO, quadEBO;

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), quadVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), quadIndi, GL_STATIC_DRAW);

    // Vertices positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
#endif

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

#if _DEBUG
    PBR = new Shader("./res/shader/basic.vert", "./res/shader/basic.frag");
#else
    PBR = new Shader("./res/shader/basic2.vert", "./res/shader/basic2.frag");
#endif

#if _DEBUG
    Camera::SetPosition(glm::vec3(-0.05f, 0.f, 0.f));
    Camera::SetRotation(glm::vec3(0.f, 180.f, 0.f));
    trans = glm::translate(trans, glm::vec3(-6.f, 0.f, 0.f));
#else
    switch (position) {
        case RenderPosition::TOP: {
            Camera::SetPosition(glm::vec3(0.f, 0.05f, 0.f));
            Camera::SetRotation(glm::vec3(90.f, 0.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, 1.2f, 0.f));
            trans = glm::rotate(trans, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
            break;
        }
        case RenderPosition::BOTTOM: {
            Camera::SetPosition(glm::vec3(0.f, -0.05f, 0.f));
            Camera::SetRotation(glm::vec3(-90.f, 0.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, -1.2f, 0.f));
            trans = glm::rotate(trans, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            break;
        }
        case RenderPosition::FRONT: {
            Camera::SetPosition(glm::vec3(0.05f, 0.f, 0.f));
            trans = glm::translate(trans, glm::vec3(1.2f, 0.f, 0.f));
            trans = glm::rotate(trans, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case RenderPosition::DEFAULT:
        case RenderPosition::BACK: {
            Camera::SetPosition(glm::vec3(-0.05f, 0.f, 0.f));
            Camera::SetRotation(glm::vec3(0.f, 180.f, 0.f));
            trans = glm::translate(trans, glm::vec3(-1.2f, 0.f, 0.f));
            break;
        }
        case RenderPosition::RIGHT: {
            Camera::SetPosition(glm::vec3(0.f, 0.f, 0.05f));
            Camera::SetRotation(glm::vec3(0.f, 90.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, 0.f, 1.2f));
            trans = glm::rotate(trans, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case RenderPosition::LEFT: {
            Camera::SetPosition(glm::vec3(0.f, 0.f, -0.05f));
            Camera::SetRotation(glm::vec3(0.f, -90.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, 0.f, -1.2f));
            trans = glm::rotate(trans, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
    }
#endif

#if _DEBUG
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Process I/O operations here
        input();

        // OpenGL rendering code here
        render();

        // Draw ImGui
        imgui_begin();
        imgui_render(); // edit this function to add your own ImGui controls
        imgui_end(); // this call effectively renders ImGui

        // End frame and swap buffers (double buffering)
        end_frame();
    }
#else
    GLuint FBO = 0, RBO = 0, resTex = 0;
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

    glGenTextures(1, &resTex);
    glBindTexture(GL_TEXTURE_2D, resTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resTex, 0);

    for (int i = 0; i < 3; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwPollEvents();
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Save Image
    int width, height;
    glBindTexture(GL_TEXTURE_2D, resTex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    unsigned char* data = new unsigned char[width * height * 3]; // GL_RGB
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    stbi_flip_vertically_on_write(true);
    std::string name = !fileName.empty() ? fileName : "PBR_Image.png";
    std::string savePath = !saveDir.empty() ? saveDir : std::filesystem::current_path().string();
    int result = stbi_write_png(std::filesystem::path(std::string(savePath).append("\\").append(name)).string().c_str(), width, height, 3, data, 0);

    if (result != 0) {
        spdlog::info("File '{}' saved in directory '{}'", name, savePath);
    }
    else {
        spdlog::error("There was an error while trying to save '{}' in directory '{}'", name, savePath);
    }

    delete[] data;

    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &resTex);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteFramebuffers(1, &FBO);
#endif

    glDeleteVertexArrays(1, &quadVAO);

#if _DEBUG
    Shape::Deinit();
#else
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadEBO);
#endif

    delete PBR;
    PBR = nullptr;
    Skybox::Deinit();

    for (int i = 0; i < 6; ++i) 
    {
        delete imageTextures[i];
        imageTextures[i] = nullptr;
    }

    glDeleteTextures(1, &defaultWhiteTexture);
    glDeleteTextures(1, &defaultBlackTexture);

#if _DEBUG
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool init()
{
    // Setup window
#if _DEBUG
    glfwSetErrorCallback(glfw_error_callback);
#endif

    if (!glfwInit()) 
    {
        spdlog::error("Failed to initalize GLFW!");
        return false;
    }
    spdlog::info("Successfully initialized GLFW!");

    // GL 4.5 + GLSL 450
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

#if !_DEBUG
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif
    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    if (window == NULL)
    {
        spdlog::error("Failed to create GLFW Window!");
        return false;
    }
    spdlog::info("Successfully created GLFW Window!");

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1); // Enable VSync - fixes FPS at the refresh rate of your screen
#if _DEBUG
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
#endif

    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (err)
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }
    spdlog::info("Successfully initialized OpenGL loader!");

#if _DEBUG
    // Debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(ErrorMessageCallback, 0);
#endif

    const GLubyte* renderer = glGetString(GL_RENDERER);
    spdlog::info("Graphic Card: {0}", (char*)renderer);

    // Depth Test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Cubemap seams
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    defaultWhiteTexture = LoadDefaultWhiteTexture();
    defaultBlackTexture = LoadDefaultBlackTexture();

    return true;
}

void render()
{
    // OpenGL Rendering code goes here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    PBR->Use();
    PBR->SetMat4("model", trans);
    PBR->SetVec3("camPos", Camera::GetPosition());

    for (int i = 0; i < 6; ++i) {
        if (imageTextures[i] != nullptr) 
            imageTextures[i]->Use(i);
        else {
            glActiveTexture(GL_TEXTURE0 + i);
            if (i >= 2 && i <= 4) glBindTexture(GL_TEXTURE_2D, defaultBlackTexture);
            else glBindTexture(GL_TEXTURE_2D, defaultWhiteTexture);
        }

        std::string name = imageName[i];
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) 
        {
            return std::tolower(c); 
        });

        PBR->SetInt(name.append("Map"), i);
    }

    PBR->SetFloat("height_scale", height_scale);

    Skybox::UseIrradianceTexture(6);
    PBR->SetInt("irradianceMap", 6);

    Skybox::UsePrefilterTexture(7);
    PBR->SetInt("prefilterMap", 7);

    Skybox::UseBrdfLUTTexture(8);
    PBR->SetInt("brdfLUT", 8);

    glBindVertexArray(quadVAO);
#if !_DEBUG
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)quadIndi);
#else
    glDrawElements(GL_TRIANGLES, Shape::GetSphereIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetSphereIndices());
#endif
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
    Skybox::Draw();
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);
}

GLuint LoadDefaultWhiteTexture()
{
    unsigned char whitePixel[3] = { 255, 255, 255 };

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

GLuint LoadDefaultBlackTexture()
{
    unsigned char blackPixel[3] = { 0, 0, 0 };

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blackPixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

#if !_DEBUG
void printHelp() {
    spdlog::set_pattern("%v");

    spdlog::info("Usage:");
    spdlog::info("  PBR_Visualiser.exe [-h] [-f <image_path1> <image_path2> ...] [-n <output_name>] [-d <directory_path>] [-p <position>] [-r <resolution>]");
    spdlog::info("Options:");
    spdlog::info("  -h                  Display this help message and exit.");
    spdlog::info("  -f <image_path>     Specify up to 6 image paths to process. Additional paths will be ignored.");
    spdlog::info("                      Example: program -f image1.jpg image2.png image3.png");
    spdlog::info("  -n <output_name>    Specify the output file name. Appends '.png' by default. Default is 'PBR_Image'.");
    spdlog::info("  -d <directory_path> Specify the directory where files will be saved. Default is the current executable path.");
    spdlog::info("  -p <position>       Specify the position of the plane in world. Accepted values: top, bottom, front, back, left, right.");
    spdlog::info("                      Default position is 'back'.");
    spdlog::info("  -r <resolution>     Specify the resolution of the output image. Accepted values: r128, r256, r512, r1k, r2k, r4k.");
    spdlog::info("                      Default resolution is 'r2k'.");
    spdlog::info("                      The dimensions of the output image for the corresponding value are: 128x128, 256x256, 512x512, 1024x1024, 2048x2048, 4096x4096.");
}

void processFileArguments(int& i, int argc, char** argv, std::vector<std::string>& imgPaths) {
    if (i >= argc) {
        spdlog::warn("The '-f' prefix was used, but no file path was specified!");
        return;
    }

    while (i < argc && imgPaths.size() < 6) {
        std::string arg = argv[i];

        // Check if the next argument is a new flag (starts with '-')
        if (arg[0] == '-') {
            --i;  // Step back to re-process this as the next option
            break;
        }

        imgPaths.push_back(std::filesystem::absolute(std::filesystem::path(arg)).string());
        ++i;
    }

    if (imgPaths.size() == 6 && i < argc && argv[i][0] != '-') {
        spdlog::warn("Too many image paths specified! Ignoring additional paths.");
        return;
    }

    if (i < argc && std::string(argv[i])[0] == '-') {
        --i;
        return;
    }
}

void processNameArgument(const std::string& arg, std::string& fileName) {
    if (!fileName.empty()) {
        spdlog::warn("File name has already been specified! Ignoring additional name.");
        return;
    }
    fileName = arg + ".png";
}

void processDirectoryArgument(const std::string& arg, std::string& saveDir) {
    try {
        std::filesystem::path dirPath = std::filesystem::absolute(arg);
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
            saveDir = dirPath.string();
        }
        else {
            spdlog::warn("Provided path after '-d' is invalid or is not a directory!");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        spdlog::warn("Error processing directory path after '-d': {}", e.what());
    }
}

void processPositionArgument(const std::string& arg, RenderPosition& position) {
    static const std::vector<std::string> validPositions = { "top", "bottom", "front", "back", "left", "right" };

    if (position != RenderPosition::DEFAULT) {
        spdlog::warn("Position has already been specified! Ignoring additional position.");
        return;
    }

    std::string pos = arg;

    std::transform(pos.begin(), pos.end(), pos.begin(), [](unsigned char c)
    {
        return std::tolower(c);
    });

    auto item = std::find(validPositions.begin(), validPositions.end(), pos);
    if (item != validPositions.end()) {
        position = (RenderPosition)(uint8_t)(item - validPositions.begin());
        return;
    }
    else {
        spdlog::warn("Invalid position argument: '{}'. Accepted values are: top, bottom, front, back, left, right.", arg);
        return;
    }
}

void processResolutionArgument(const std::string& arg, RenderResolution& resolution) {
    static const std::vector<std::string> validResolutions = { "r128", "r256", "r512", "r1k", "r2k", "r4k" };

    if (resolution != RenderResolution::DEFAULT) {
        spdlog::warn("Resolution has already been specified! Ignoring additional resolution.");
        return;
    }

    std::string res = arg;

    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c)
        {
            return std::tolower(c);
        });

    auto item = std::find(validResolutions.begin(), validResolutions.end(), res);
    if (item != validResolutions.end()) {
        resolution = (RenderResolution)(uint8_t)(item - validResolutions.begin());
        return;
    }
    else {
        spdlog::warn("Invalid resolution argument: '{}'. Accepted values are: r128, r256, r512, r1k, r2k, r4k.", arg);
        return;
    }
}
#else
void end_frame()
{
    TimeManager::Update();
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

void input()
{
    bool pressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() + Camera::GetFrontDir() * cameraSpeed * TimeManager::GetDeltaTime());
        pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() - Camera::GetFrontDir() * cameraSpeed * TimeManager::GetDeltaTime());
        pressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() - Camera::GetRight() * cameraSpeed * TimeManager::GetDeltaTime());
        pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        Camera::SetPosition(Camera::GetPosition() + Camera::GetRight() * cameraSpeed * TimeManager::GetDeltaTime());
        pressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && released) {

        released = false;
        mouseNotUsed = true;
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPosCallback(window, mouse_callback);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_REPEAT && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) {
        released = true;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (mouseNotUsed)
    {
        lastX = xpos;
        lastY = ypos;
        mouseNotUsed = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    glm::vec3 rot = Camera::GetRotation();

    // YAW = ROT Y
    // PITCH = ROT X
    // ROLL = ROT Z

    rot.x += yoffset;

    if (rot.x > 89.f) {
        rot.x = 89.f;
    }

    if (rot.x < -89.f)
    {
        rot.x = -89.f;
    }

    Camera::SetRotation(glm::vec3(rot.x, rot.y + xoffset, rot.z));
}

void init_imgui()
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    io.ConfigDockingTransparentPayload = true;  // Enable Docking Transparent

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
}

void imgui_begin()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_render()
{
    static bool _skyboxOpen = true;

    if (!ImGui::Begin("PBR VISUALISER", nullptr, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Windows##Menu")) {
            if (ImGui::MenuItem("Skybox")) {
                _skyboxOpen = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    for (int i = 0; i < 6; ++i)
    {
        if (ImGui::Button(("Load Image " + imageName[i]).c_str()))
        {
            openFileDialogs[i] = true;
            fileDialogInfos[i].title = "Choose " + imageName[i] + " image";
            fileDialogInfos[i].type = ImGuiFileDialogType_OpenFile;
            fileDialogInfos[i].directoryPath = std::filesystem::current_path().append("./res/textures/");
        }

        if (openFileDialogs[i])
        {
            if (ImGui::FileDialog(&openFileDialogs[i], &fileDialogInfos[i]))
            {
                TextureFileFormat inter = i == 0 ? TextureFileFormat::SRGB : i == 1 ? TextureFileFormat::RGB : TextureFileFormat::RED;
                TextureFormat form = i == 0 ? TextureFormat::RGB : i == 1 ? TextureFormat::RGB : TextureFormat::RED;
                imageTextures[i] = new Texture2D(fileDialogInfos[i].resultPath.string().c_str(), inter, form);
            }
        }

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130);

        if (imageTextures[i] != nullptr)
        {
            ImGui::Image((void*)(intptr_t)imageTextures[i]->GetId(), ImVec2(128, 128));
        }
        else 
        {
            ImGui::Image((void*)(intptr_t)(i >= 2 && i <= 4 ? defaultBlackTexture : defaultWhiteTexture), ImVec2(128, 128));
        }
    }

    ImGui::DragFloat("Height", &height_scale, 0.1f, 0.0f, FLT_MAX);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    if (_skyboxOpen) Skybox::DrawEditor(&_skyboxOpen);
}

void imgui_end()
{
    ImGui::Render();
    glfwMakeContextCurrent(window);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
#endif