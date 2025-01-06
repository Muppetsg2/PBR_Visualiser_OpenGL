//
//     ___  ___  ___    _   ___               ___            
//    / _ \/ _ )/ _ \  | | / (_)__ __ _____ _/ (_)__ ___ ____
//   / ___/ _  / , _/  | |/ / (_-</ // / _ `/ / (_-</ -_) __/
//  /_/  /____/_/|_|   |___/_/___/\_,_/\_,_/_/_/___/\__/_/   
//
// Version: 1.3.0
// Author: Marceli Antosik
// Last Update: 07.01.2025

extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#include <Texture2D.h>
#include <Shader.h>
#include <Skybox.h>
#include <Camera.h>
#include <ShadersExtractor.h>

#if WINDOW_APP
#include <TimeManager.h>
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
void drawBanner();
void render();
GLuint LoadDefaultWhiteTexture();
GLuint LoadDefaultBlackTexture();

#if !WINDOW_APP
ENUM_CLASS_BASE_VALUE(RenderPosition, uint8_t, TOP, 0, BOTTOM, 1, FRONT, 2, BACK, 3, RIGHT, 4, LEFT, 5, DEFAULT, 6)

ENUM_CLASS_BASE_VALUE(RenderResolution, uint8_t, R128, 0, R256, 1, R512, 2, R1K, 3, R2K, 4, R4K, 5, DEFAULT, 6)

ENUM_CLASS_BASE_VALUE(SkyboxEnum, uint8_t, PARK, 0, HILL, 1, PHOTOSTUDIO, 2, BATHROOM, 3, MOONLESS_GOLF, 4, SNOWY_FIELD, 5, VENICE_SUNSET, 6, SATARA_NIGHT, 7, DEFAULT, 8)

bool isExposureSet = false;
bool isIntensitySet = false;

void printHelp();
void processFileArguments(int& i, int argc, char** argv, std::vector<std::string>& imgPaths);
void processSkyArgument(const std::string& arg, SkyboxEnum& sky);
void processNameArgument(const std::string& arg, std::string& fileName);
void processDirectoryArgument(const std::string& arg, std::string& direc);
void processPositionArgument(const std::string& arg, RenderPosition& position);
void processResolutionArgument(const std::string& arg, RenderResolution& resolution);
void processExposureArgument(const std::string& arg, float& expValue, bool& isValueSet);
void processIntensityArgument(const std::string& arg, float& intensityValue, bool& isValueSet);
#else
ENUM_CLASS_BASE_VALUE(ShapeType, uint8_t, Sphere, 0, Cube, 1, Plane, 2)

ENUM_CLASS_BASE_VALUE(PlaneNormalOrientation, uint8_t, TOP, 0, BOTTOM, 1, FRONT, 2, BACK, 3, RIGHT, 4, LEFT, 5)

ShapeType shapeType = ShapeType::Sphere;
PlaneNormalOrientation planeNormalOrientation = PlaneNormalOrientation::TOP;

void set_shape(GLuint VAO, ShapeType type);
void set_plane_normal_orientation(PlaneNormalOrientation orientation);
glm::mat4 get_plane_normal_orientation_mat(glm::mat4 planeTrans, PlaneNormalOrientation orientation);
void end_frame();
void input();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void init_imgui();
void imgui_begin();
void imgui_render();
void imgui_end();
#endif

constexpr const char* WINDOW_NAME = "PBR_Visualiser";
#if WINDOW_APP
constexpr int32_t WINDOW_WIDTH = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;
#else
int32_t WINDOW_WIDTH = 2048;
int32_t WINDOW_HEIGHT = 2048;
#endif

GLFWwindow* window = nullptr;
std::string exeDirPath;

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
float exposure = 1.f;
float colorIntensity = 1.f;

#if !WINDOW_APP
std::vector<std::string> skyboxPaths =
{
    "/res/skybox/rooitou_park_4k.hdr",
    "/res/skybox/hilly_terrain_01_4k.hdr",
    "/res/skybox/brown_photostudio_01_4k.hdr",
    "/res/skybox/modern_bathroom_4k.hdr",
    "/res/skybox/moonless_golf_4k.hdr",
    "/res/skybox/snowy_field_4k.hdr",
    "/res/skybox/venice_sunset_4k.hdr",
    "/res/skybox/satara_night_4k.hdr"
};

glm::vec3 quadVerts[4] = {
    { 0.f, -.5f, -.5f },
    { 0.f, -.5f, .5f  },
    { 0.f, .5f, -.5f  },
    { 0.f, .5f,  .5f  }
};

unsigned int quadIndi[6] = {
    0, 1, 2,
    1, 3, 2
};
#endif

#if WINDOW_APP
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
    exeDirPath = std::filesystem::absolute(argv[0]).parent_path().string();

    ShadersExtractor::Init(exeDirPath + "\\shaders.dat");

#if WINDOW_APP
    drawBanner();
#else

    std::string fileName;
    std::string saveDir;
    std::vector<std::string> imgPaths;
    SkyboxEnum sky = SkyboxEnum::DEFAULT;
    RenderPosition position = RenderPosition::DEFAULT;
    RenderResolution resolution = RenderResolution::DEFAULT;

    if (argc > 1) {
        bool expectSky = false;
        bool expectName = false;
        bool expectDirectory = false;
        bool expectPosition = false;
        bool expectResolution = false;
        bool expectExposure = false;
        bool expectIntensity = false;

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
            else if (expectSky) {
                processSkyArgument(arg, sky);
                expectSky = false;
            }
            else if (expectResolution) {
                processResolutionArgument(arg, resolution);
                expectResolution = false;
            }
            else if (expectExposure) {
                processExposureArgument(arg, exposure, isExposureSet);
                expectExposure = false;
            }
            else if (expectIntensity) {
                processIntensityArgument(arg, colorIntensity, isIntensitySet);
                expectIntensity = false;
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
            else if (arg == "-s") {
                expectSky = true;
            }
            else if (arg == "-r") {
                expectResolution = true;
            }
            else if (arg == "-e") {
                expectExposure = true;
            }
            else if (arg == "-i") {
                expectIntensity = true;
            }
            else if (arg == "-v") {
                Config::setVerbose(true);
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
        if (expectSky) {
            spdlog::warn("The '-s' prefix was used, but no skybox was specified!");
        }
        if (expectResolution) {
            spdlog::warn("The '-r' prefix was used, but no resolution was specified!");
        }
        if (expectExposure) {
            spdlog::warn("The '-e' prefix was used, but no exposure was specified!");
        }
        if (expectIntensity) {
            spdlog::warn("The '-i' prefix was used, but no color intensity was specified!");
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

    if (Config::isVerbose()) spdlog::info("Initialized project.");

#if WINDOW_APP
    init_imgui();
    spdlog::info("Initialized ImGui.");

    Camera::Init(window);
#else
    Camera::Init(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT));
#endif

#if WINDOW_APP
    Skybox::Init(window, /*exeDirPath,*/ "./res/skybox/rooitou_park_4k.hdr");
#else

    spdlog::info("Skybox: {}", to_string(sky));

    Skybox::Init(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT), /*exeDirPath,*/ std::string(exeDirPath + skyboxPaths[(uint8_t)(sky != SkyboxEnum::DEFAULT ? sky : SkyboxEnum::PARK)]).c_str());

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

#if WINDOW_APP
    set_shape(quadVAO, shapeType);
    set_plane_normal_orientation(planeNormalOrientation);
#else
    glBindVertexArray(quadVAO);

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#endif

#if WINDOW_APP
    //PBR = new Shader(std::string(exeDirPath + "/res/shader/basic.vert").c_str(), std::string(exeDirPath + "/res/shader/basic.frag").c_str());
    PBR = Shader::FromExtractor("basic.vert", "basic.frag");
#else
    //PBR = new Shader(std::string(exeDirPath + "/res/shader/basic2.vert").c_str(), std::string(exeDirPath + "/res/shader/basic2.frag").c_str());
    PBR = Shader::FromExtractor("basic2.vert", "basic2.frag");
#endif

#if WINDOW_APP
    Camera::SetPosition(glm::vec3(-0.05f, 0.f, 0.f));
    Camera::SetRotation(glm::vec3(0.f, 180.f, 0.f));
    trans = glm::translate(trans, glm::vec3(-6.f, 0.f, 0.f));
#else
    switch (position) {
        case RenderPosition::TOP: {
            Camera::SetPosition(glm::vec3(0.f, -0.05f, 0.f));
            Camera::SetRotation(glm::vec3(-90.f, 0.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, -1.2f, 0.f));
            trans = glm::rotate(trans, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            break;
        }
        case RenderPosition::BOTTOM: {
            Camera::SetPosition(glm::vec3(0.f, 0.05f, 0.f));
            Camera::SetRotation(glm::vec3(90.f, 0.f, 0.f));
            trans = glm::translate(trans, glm::vec3(0.f, 1.2f, 0.f));
            trans = glm::rotate(trans, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
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

    spdlog::info("Position: {}", to_string(position));
    spdlog::info("Exposure: {}", exposure);
    spdlog::info("Color Intensity: {}", colorIntensity);

#endif

#if WINDOW_APP

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Process I/O operations here
        input();

        Camera::StartCapturing();
        // OpenGL rendering code here
        render();
        Camera::StopCapturing();

        Camera::Render();

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

#if WINDOW_APP
    Shape::Deinit();
#else
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadEBO);
#endif

    delete PBR;
    PBR = nullptr;
    Skybox::Deinit();

    Camera::Deinit();

    for (int i = 0; i < 6; ++i) 
    {
        delete imageTextures[i];
        imageTextures[i] = nullptr;
    }

    glDeleteTextures(1, &defaultWhiteTexture);
    glDeleteTextures(1, &defaultBlackTexture);

    ShadersExtractor::Deinit();

#if WINDOW_APP
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
    
    if (Config::isVerbose()) spdlog::info("Successfully initialized GLFW!");

    // GL 4.5 + GLSL 450
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

#if !WINDOW_APP
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif
    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    if (window == NULL)
    {
        spdlog::error("Failed to create GLFW Window!");
        return false;
    }

    if (Config::isVerbose()) spdlog::info("Successfully created GLFW Window!");

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

    if (Config::isVerbose()) spdlog::info("Successfully initialized OpenGL loader!");

#if _DEBUG
    // Debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(ErrorMessageCallback, 0);
#endif

    if (Config::isVerbose()) {
        const GLubyte* renderer = glGetString(GL_RENDERER);
        spdlog::info("Graphic Card: {0}", (char*)renderer);
    }

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

void drawBanner()
{
    spdlog::set_pattern("%v");

    spdlog::info("     ___  ___  ___    _   ___               ___              ");
    spdlog::info("    / _ \\/ _ )/ _ \\  | | / (_)__ __ _____ _/ (_)__ ___ ____");
    spdlog::info("   / ___/ _  / , _/  | |/ / (_-</ // / _ `/ / (_-</ -_) __/  ");
    spdlog::info("  /_/  /____/_/|_|   |___/_/___/\\_,_/\\_,_/_/_/___/\\__/_/  \n");
    spdlog::info("                       Version: {}", PBR_VISUALISER_VERSION_STR);
    spdlog::info("                   Author: Marceli Antosik");
    spdlog::info("                   Last Update: {}", PBR_VISUALISER_LAST_UPDATE);

#if WINDOW_APP && !_DEBUG
    spdlog::info("                    Configuration: WINDOW\n");
#elif WINDOW_APP && _DEBUG
    spdlog::info("                  Configuration: WINDOW DEBUG\n");
#elif !WINDOW_APP && !_DEBUG
    spdlog::info("                    Configuration: CONSOLE\n");
#elif !WINDOW_APP && _DEBUG
    spdlog::info("                  Configuration: CONSOLE DEBUG\n");
#endif

    // Default spdlog pattern
    // [2014-10-31 23:46:59.678] [my_loggername] [info] Some message
    spdlog::set_pattern("%+");
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
    PBR->SetFloat("exposure", exposure);
    PBR->SetFloat("colorIntensity", colorIntensity);

    Skybox::UseIrradianceTexture(6);
    PBR->SetInt("irradianceMap", 6);

    Skybox::UsePrefilterTexture(7);
    PBR->SetInt("prefilterMap", 7);

    Skybox::UseBrdfLUTTexture(8);
    PBR->SetInt("brdfLUT", 8);

    glBindVertexArray(quadVAO);
#if !WINDOW_APP
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)quadIndi);
#else
    switch (shapeType) {
        case ShapeType::Sphere: {
            glDrawElements(GL_TRIANGLES, Shape::GetSphereIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetSphereIndices());
            break;
        }
        case ShapeType::Cube: {
            glDrawElements(GL_TRIANGLES, Shape::GetCubeIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetCubeIndices());
            break;
        }
        case ShapeType::Plane: {
            glm::mat4 pTrans = get_plane_normal_orientation_mat(trans, planeNormalOrientation);
            PBR->SetMat4("model", pTrans);
            glDrawElements(GL_TRIANGLES, Shape::GetQuadIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetQuadIndices());
            break;
        }
        default: {
            glDrawElements(GL_TRIANGLES, Shape::GetSphereIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetSphereIndices());
        }
    }
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

#if !WINDOW_APP
void printHelp() {

    drawBanner();

    spdlog::set_pattern("%v");

    spdlog::info("Usage:");
    spdlog::info("  PBR_Visualiser.exe ([-h] | [-v] [-f <albedo_path> <normal_path> ...] [-n <output_name>] [-d <directory_path>] [-p <position>] [-s <skybox>] [-r <resolution>] [-e <exposure_value>] [-i <color_intensity>])");
    spdlog::info("Options:");
    spdlog::info("  -h                   Display this help message and exit.");
    spdlog::info("  -v                   Throws more detailed information as output to the console");
    spdlog::info("  -f <image_path>      Specify up to 6 image paths to process. Additional paths will be ignored.");
    spdlog::info("                       The paths must be in the order: albedo, normal, metallness, displacement, roughness, ambient occlusion.");
    spdlog::info("                       Example: program -f albedo_image.jpg normal_image.png metalness_image.png");
    spdlog::info("  -n <output_name>     Specify the output file name. Appends '.png' by default. Default is 'PBR_Image'.");
    spdlog::info("  -d <directory_path>  Specify the directory where files will be saved. Default is the current executable path.");
    spdlog::info("  -p <position>        Specify the position of the plane in world. Accepted values: top, bottom, front, back, right, left.");
    spdlog::info("                       Default position is 'back'.");
    spdlog::info("  -s <skybox>          Specify the skybox texture. Accepted values: park, hill, photostudio, bathroom, moonless_golf, snowy_field, venice_sunset, satara_night.");
    spdlog::info("                       Default skybox texture is 'park'.");
    spdlog::info("  -r <resolution>      Specify the resolution of the output image. Accepted values: r128, r256, r512, r1k, r2k, r4k.");
    spdlog::info("                       Default resolution is 'r2k'.");
    spdlog::info("                       The dimensions of the output image for the corresponding value are: 128x128, 256x256, 512x512, 1024x1024, 2048x2048, 4096x4096.");
    spdlog::info("  -e <exposure_value>  Specify the exposure value of output image. Value must be in valid float format within range (0 - 11). Default value is 1.0.");
    spdlog::info("  -i <color_intensity> Specify the color intensity value of output image. Value must be in valid float format within range (0 - 4). Default value is 1.0.");
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
            return;
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

void processSkyArgument(const std::string& arg, SkyboxEnum& sky)
{
    static const std::vector<std::string> validSkyboxes = { "park", "hill", "photostudio", "bathroom", "moonless_golf", "snowy_field", "venice_sunset", "satara_night" };

    if (sky != SkyboxEnum::DEFAULT) {
        spdlog::warn("Skybox has already been specified! Ignoring additional resolution.");
        return;
    }

    std::string res = arg;

    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c)
        {
            return std::tolower(c);
        });

    auto item = std::find(validSkyboxes.begin(), validSkyboxes.end(), res);
    if (item != validSkyboxes.end()) {
        sky = (SkyboxEnum)(uint8_t)(item - validSkyboxes.begin());
        return;
    }
    else {
        spdlog::warn("Invalid skybox argument: '{}'. Accepted values are: park, hill, photostudio, bathroom, moonless_golf, snowy_field, venice_sunset, satara_night.", arg);
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
    static const std::vector<std::string> validPositions = { "top", "bottom", "front", "back", "right", "left" };

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
        spdlog::warn("Invalid position argument: '{}'. Accepted values are: top, bottom, front, back, right, left.", arg);
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

void processExposureArgument(const std::string& arg, float& expValue, bool& isValueSet) {

    const float MIN_EXPOSURE_VALUE = 0.0f;
    const float MAX_EXPOSURE_VALUE = 11.0f;

    if (isValueSet) {
        spdlog::warn("Exposure value has already been specified and is valid! Ignoring additional input.");
        return;
    }

    std::istringstream iss(arg);
    float extractedValue;
    if (!(iss >> extractedValue)) {
        spdlog::warn("Invalid exposure float format in input string: '{}'. Expected only a valid float value.", arg);
        return;
    }

    // Consume any remaining whitespace in the stream
    iss >> std::ws;
    if (iss.peek() != EOF) {
        spdlog::warn("Input exposure string contains invalid trailing characters: '{}'. Expected only a valid float value.", arg);
        return;
    }

    if (extractedValue < MIN_EXPOSURE_VALUE || extractedValue > MAX_EXPOSURE_VALUE) {
        spdlog::warn("Exposure value out of range ({} - {}): {}", MIN_EXPOSURE_VALUE, MAX_EXPOSURE_VALUE, extractedValue);
        return;
    }

    expValue = extractedValue;
    isValueSet = true;
}

void processIntensityArgument(const std::string& arg, float& intensityValue, bool& isValueSet) {

    const float MIN_INTENSITY_VALUE = 0.0f;
    const float MAX_INTENSITY_VALUE = 4.0f;

    if (isValueSet) {
        spdlog::warn("Color intensity value has already been specified and is valid! Ignoring additional input.");
        return;
    }

    std::istringstream iss(arg);
    float extractedValue;
    if (!(iss >> extractedValue)) {
        spdlog::warn("Invalid color intensity float format in input string: '{}'. Expected only a valid float value.", arg);
        return;
    }

    // Consume any remaining whitespace in the stream
    iss >> std::ws;
    if (iss.peek() != EOF) {
        spdlog::warn("Input color intensity string contains invalid trailing characters: '{}'. Expected only a valid float value.", arg);
        return;
    }

    if (extractedValue < MIN_INTENSITY_VALUE || extractedValue > MAX_INTENSITY_VALUE) {
        spdlog::warn("Color intensity value out of range ({} - {}): {}", MIN_INTENSITY_VALUE, MAX_INTENSITY_VALUE, extractedValue);
        return;
    }

    intensityValue = extractedValue;
    isValueSet = true;
}
#else
void set_shape(GLuint VAO, ShapeType type)
{
    glBindVertexArray(VAO);

    switch (type) {
        case ShapeType::Sphere: {
            glBindBuffer(GL_ARRAY_BUFFER, Shape::GetSphereVBO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetSphereEBO());
            break;
        }
        case ShapeType::Cube: {
            glBindBuffer(GL_ARRAY_BUFFER, Shape::GetCubeVBO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetCubeEBO());
            break;
        }
        case ShapeType::Plane: {
            glBindBuffer(GL_ARRAY_BUFFER, Shape::GetQuadVBO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetQuadEBO());
            break;
        }
        default: {
            glBindBuffer(GL_ARRAY_BUFFER, Shape::GetSphereVBO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::GetSphereEBO());
        }
    }

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    spdlog::info("Shape Type: {}", to_string(type));

    shapeType = type;
}

void set_plane_normal_orientation(PlaneNormalOrientation orientation)
{
    spdlog::info("Plane Normal Orientation: {}", to_string(orientation));

    planeNormalOrientation = orientation;
}

glm::mat4 get_plane_normal_orientation_mat(glm::mat4 planeTrans, PlaneNormalOrientation orientation)
{
    switch (orientation) {
        case PlaneNormalOrientation::TOP: {
            planeTrans = glm::rotate(planeTrans, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case PlaneNormalOrientation::BOTTOM: {
            planeTrans = glm::rotate(planeTrans, glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
            planeTrans = glm::rotate(planeTrans, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case PlaneNormalOrientation::FRONT: {
            planeTrans = glm::rotate(planeTrans, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
            planeTrans = glm::rotate(planeTrans, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case PlaneNormalOrientation::BACK: {
            planeTrans = glm::rotate(planeTrans, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            planeTrans = glm::rotate(planeTrans, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case PlaneNormalOrientation::RIGHT: {
            planeTrans = glm::rotate(planeTrans, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            planeTrans = glm::rotate(planeTrans, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        }
        case PlaneNormalOrientation::LEFT: {
            planeTrans = glm::rotate(planeTrans, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
            break;
        }
    }

    return planeTrans;
}

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

    if (ImGui::Button("Save Screenshot")) {

        std::string screenFolderPath = std::string(exeDirPath).append("\\").append("Screenshots");

        bool exist = false;
        struct stat info;
        if (stat(screenFolderPath.c_str(), &info) != 0) {
            exist = false;  // Folder nie istnieje
        }
        else if (info.st_mode & S_IFDIR) {
            exist = true;   // Istnieje i jest folderem
        }

        if (_mkdir(screenFolderPath.c_str()) == 0) {
            spdlog::info("Directory '{}' has been created!", screenFolderPath);
            exist = true;
        }
        else {
            spdlog::error("Directory '{}' couldn't have been created!", screenFolderPath);
            exist = false;
        }

        if (exist) Camera::SaveScreenshot(screenFolderPath);
        else spdlog::error("Failed to save screenshot!");
    }

    if (ImGui::BeginCombo("Shape", to_string(shapeType).c_str()))
    {
        for (size_t i = 0; i < size<ShapeType>(); ++i) {
            ShapeType acc = (ShapeType)i;
            if (ImGui::Selectable(to_string(acc).c_str(), shapeType == acc))
            {
                set_shape(quadVAO, acc);
                break;
            }
        }
        ImGui::EndCombo();
    }

    if (shapeType == ShapeType::Plane) 
    {
        if (ImGui::BeginCombo("Plane Normal Orientation", to_string(planeNormalOrientation).c_str()))
        {
            for (size_t i = 0; i < size<PlaneNormalOrientation>(); ++i) {
                PlaneNormalOrientation acc = (PlaneNormalOrientation)i;
                if (ImGui::Selectable(to_string(acc).c_str(), planeNormalOrientation == acc))
                {
                    set_plane_normal_orientation(acc);
                    break;
                }
            }
            ImGui::EndCombo();
        }
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
            ImGui::Image((intptr_t)imageTextures[i]->GetId(), ImVec2(128, 128));
        }
        else 
        {
            ImGui::Image((intptr_t)(i >= 2 && i <= 4 ? defaultBlackTexture : defaultWhiteTexture), ImVec2(128, 128));
        }
    }

    ImGui::DragFloat("Height", &height_scale, 0.1f, 0.0f, FLT_MAX);
    ImGui::SliderFloat("Exposure", &exposure, 0.0f, 11.0f, "Exposure: %.2f");
    ImGui::SliderFloat("Color Intensity", &colorIntensity, 0.0f, 4.0f, "Intensity: %.2f");

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