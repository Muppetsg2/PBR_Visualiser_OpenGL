//
//     ___  ___  ___    _   ___               ___            
//    / _ \/ _ )/ _ \  | | / (_)__ __ _____ _/ (_)__ ___ ____
//   / ___/ _  / , _/  | |/ / (_-</ // / _ `/ / (_-</ -_) __/
//  /_/  /____/_/|_|   |___/_/___/\_,_/\_,_/_/_/___/\__/_/   
//
// Version: 1.3.9
// Author: Marceli Antosik (Muppetsg2)
// Last Update: 22.04.2025

extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#include <Texture2D.h>
#include <Shader.h>
#include <Skybox.h>
#include <Camera.h>
#include <ShadersExtractor.h>
#include <Shape.h>

#if WINDOW_APP
#include <TimeManager.h>
#include <ModelLoader.h>
#else
#include <ArgumentParser.h>
#endif

#if WINDOW_APP && _DEBUG
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
#elif WINDOW_APP && !_DEBUG
static void GLAPIENTRY ErrorMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    std::string severityS = "";
    if (severity == GL_DEBUG_SEVERITY_HIGH) severityS = "HIGHT";
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) severityS = "MEDIUM";
    else if (severity == GL_DEBUG_SEVERITY_LOW) severityS = "LOW";
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) severityS = "NOTIFICATION";

    if (type == GL_DEBUG_TYPE_ERROR) {
        SPDLOG_ERROR("GL CALLBACK: type = ERROR, severity = {0}, message = {1}\n", severityS, message);
    }
}
#elif !WINDOW_APP
static void GLAPIENTRY ErrorMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::string severityS = "";
    if (severity == GL_DEBUG_SEVERITY_HIGH) severityS = "HIGHT";
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) severityS = "MEDIUM";
    else if (severity == GL_DEBUG_SEVERITY_LOW) severityS = "LOW";
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && Config::IsVerbose()) severityS = "NOTIFICATION";
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && !Config::IsVerbose()) return;

    if (type == GL_DEBUG_TYPE_ERROR) {
        SPDLOG_ERROR("GL CALLBACK: type = ERROR, severity = {0}, message = {1}\n", severityS, message);
    }
    else if (type == GL_DEBUG_TYPE_MARKER) {
        if (!Config::IsVerbose()) return;
        SPDLOG_INFO("GL CALLBACK: type = MARKER, severity = {0}, message = {1}\n", severityS, message);
    }
    else {
        if (!Config::IsVerbose()) return;
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
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Camera::OnWindowSizeChange();
}

bool init();
std::string get_executable_path();
void drawBanner();
void render();
GLuint LoadDefaultWhiteTexture();
GLuint LoadDefaultBlackTexture();

#if WINDOW_APP
ENUM_CLASS_BASE_VALUE(ShapeType, uint8_t, Sphere, 0, Cube, 1, Plane, 2, Loaded_Model, 3)

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
void drawHelp(bool* open);
void imgui_end();
#else
ENUM_CLASS_BASE_VALUE(RenderPosition, uint8_t, TOP, 0, BOTTOM, 1, FRONT, 2, BACK, 3, RIGHT, 4, LEFT, 5, DEFAULT, 6)

ENUM_CLASS_BASE_VALUE(RenderResolution, uint8_t, R128, 0, R256, 1, R512, 2, R1K, 3, R2K, 4, R4K, 5, DEFAULT, 6)

ENUM_CLASS_BASE_VALUE(SkyboxEnum, uint8_t, PARK, 0, HILL, 1, PHOTOSTUDIO, 2, BATHROOM, 3, MOONLESS_GOLF, 4, SNOWY_FIELD, 5, VENICE_SUNSET, 6, SATARA_NIGHT, 7, GOLDEN_BAY, 8, DEFAULT, 9)

bool isExposureSet = false;
bool isIntensitySet = false;

// ADDITIONAL FUNCTIONS
bool isValidFileNameWindows(const std::string& name);

// PROCESS ARGUMENTS
void printHelp();
void processFileArguments(int& i, int argc, char** argv, std::vector<std::string>& imgPaths);
void processSkyArgument(const std::string& arg, SkyboxEnum& sky);
void processNameArgument(const std::string& arg, std::string& fileName);
void processDirectoryArgument(const std::string& arg, std::string& direc);
void processPositionArgument(const std::string& arg, RenderPosition& position);
void processResolutionArgument(const std::string& arg, RenderResolution& resolution);
void processExposureArgument(const std::string& arg, float& expValue, bool& isValueSet);
void processIntensityArgument(const std::string& arg, float& intensityValue, bool& isValueSet);

// PROCESS INPUT (Interacive Mode Only)
void processFileInput(std::vector<std::string>& imgPaths);
void processSkyInput(SkyboxEnum& sky);
void processNameInput(std::string& fileName);
void processDirectoryInput(std::string& direc, std::string& last);
void processPositionInput(RenderPosition& position);
void processResolutionInput(RenderResolution& resolution);
void processExposureInput(float& expValue, bool& isValueSet);
void processIntensityInput(float& intensityValue, bool& isValueSet);

// INTERPRET VALUES
void interpretFileValues(std::vector<std::string>& imgPaths);
std::string interpretSkyValue(SkyboxEnum& sky);
void interpretPositionValue(RenderPosition& position);
void interpretResolutionValue(RenderResolution& resolution);

// MAIN FUNCTIONS
void generateAndSaveImage(const std::string& fileName, const std::string& saveDir);
void interactiveModeLoop(std::string fileName, std::string saveDir, std::vector<std::string> imgPaths, SkyboxEnum sky, RenderPosition position, RenderResolution resolution);
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

const char* iconPath = "icon.png";

Texture2D* imageTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
static const std::string imageName[6] = { "Albedo", "Normal", "Metallic", "Displacement", "Roughness", "AO" };
GLuint defaultWhiteTexture = 0;
GLuint defaultBlackTexture = 0;

GLuint quadVAO = 0;
Shader* PBR = nullptr;
glm::mat4 trans = glm::mat4(1.f);
float height_scale = 0.04f;
float exposure = 1.f;
float colorIntensity = 1.f;

#if WINDOW_APP
bool drawSkybox = true;
ImVec4 bgColor = ImVec4(0.f, 0.f, 0.f, 1.f);

float cameraSpeed = 40.f;
bool released = true;
bool mouseNotUsed = true;
float sensitivity = 0.1f;

GLfloat lastX = 0.f, lastY = 0.f;
float rotateAngle = 50.f;
#else
static const std::vector<std::string> skyboxPaths =
{
    "\\res\\skybox\\rooitou_park_4k.hdr",
    "\\res\\skybox\\hilly_terrain_01_4k.hdr",
    "\\res\\skybox\\brown_photostudio_01_4k.hdr",
    "\\res\\skybox\\modern_bathroom_4k.hdr",
    "\\res\\skybox\\moonless_golf_4k.hdr",
    "\\res\\skybox\\snowy_field_4k.hdr",
    "\\res\\skybox\\venice_sunset_4k.hdr",
    "\\res\\skybox\\satara_night_4k.hdr",
    "\\res\\skybox\\golden_bay_4k.hdr"
};
#endif

int main(int argc, char** argv)
{
    exeDirPath = get_executable_path();

    ShadersExtractor::Init(exeDirPath + "\\shaders.dat");

#if WINDOW_APP
    drawBanner();

    spdlog::info("Resolution: {}x{}", WINDOW_WIDTH, WINDOW_HEIGHT);
#else

    std::string fileName;
    std::string saveDir;
    std::vector<std::string> imgPaths;
    SkyboxEnum sky = SkyboxEnum::DEFAULT;
    RenderPosition position = RenderPosition::DEFAULT;
    RenderResolution resolution = RenderResolution::DEFAULT;

    ArgumentParser::AddOption("-h", [&](const std::string&) {
        printHelp();
        imgPaths.clear();
        exit(EXIT_SUCCESS);
    });
    ArgumentParser::AddOptionWithIndex("-f", [&](int& i, int argc, char* argv[]) { ++i; processFileArguments(i, argc, argv, imgPaths); });
    ArgumentParser::AddOption("-n", [&](const std::string& value) { processNameArgument(value, fileName); }, true);
    ArgumentParser::AddOption("-d", [&](const std::string& value) { processDirectoryArgument(value, saveDir); }, true);
    ArgumentParser::AddOption("-p", [&](const std::string& value) { processPositionArgument(value, position); }, true);
    ArgumentParser::AddOption("-s", [&](const std::string& value) { processSkyArgument(value, sky); }, true);
    ArgumentParser::AddOption("-r", [&](const std::string& value) { processResolutionArgument(value, resolution); }, true);
    ArgumentParser::AddOption("-e", [&](const std::string& value) { processExposureArgument(value, exposure, isExposureSet); }, true);
    ArgumentParser::AddOption("-i", [&](const std::string& value) { processIntensityArgument(value, colorIntensity, isIntensitySet); }, true);
    ArgumentParser::AddOption("-v", [&](const std::string&) { Config::SetVerbose(true); });
    ArgumentParser::AddOption("-I", [&](const std::string&) { Config::SetInteractive(true); });

    ArgumentParser::Parse(argc, argv);

    interpretResolutionValue(resolution);

    if (!Config::IsInteractive()) spdlog::info("Resolution: {}x{}", WINDOW_WIDTH, WINDOW_HEIGHT);
#endif

    if (!init())
    {
        spdlog::error("Failed to initialize project!");
        return EXIT_FAILURE;
    }

    if (Config::IsVerbose()) spdlog::info("Initialized project.");

#if WINDOW_APP
    init_imgui();
    spdlog::info("Initialized ImGui.");

    Camera::Init(window);
#else
    Camera::Init(window);
#endif

#if WINDOW_APP
    Skybox::Init(window, std::string(exeDirPath + "\\res\\skybox\\rooitou_park_4k.hdr").c_str());
#else

    if (!Config::IsInteractive()) spdlog::info("Skybox: {}", to_string(sky));

    Skybox::Init(window, interpretSkyValue(sky).c_str());

    if (!Config::IsInteractive()) {
        interpretFileValues(imgPaths);
        imgPaths.clear();
    }
#endif

    glGenVertexArrays(1, &quadVAO);

#if WINDOW_APP
    set_shape(quadVAO, shapeType);
    set_plane_normal_orientation(planeNormalOrientation);
#endif

#if WINDOW_APP
    PBR = Shader::FromExtractor("basic.vert", "basic.frag");
#else
    PBR = Shader::FromExtractor("basic2.vert", "basic2.frag");
#endif

#if WINDOW_APP
    Camera::SetPosition(glm::vec3(-0.05f, 0.f, 0.f));
    Camera::SetRotation(glm::vec3(0.f, 180.f, 0.f));
    trans = glm::translate(trans, glm::vec3(-6.f, 0.f, 0.f));
#else
    interpretPositionValue(position);

    if (!Config::IsInteractive()) spdlog::info("Position: {}", to_string(position));
    if (!Config::IsInteractive()) spdlog::info("Exposure: {}", exposure);
    if (!Config::IsInteractive()) spdlog::info("Color Intensity: {}", colorIntensity);

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
    if (Config::IsInteractive()) {
        interactiveModeLoop(fileName, saveDir, imgPaths, sky, position, resolution);
        spdlog::set_pattern("%+");
    }
    else {
        generateAndSaveImage(fileName, saveDir);
    }
#endif

    glDeleteVertexArrays(1, &quadVAO);

    delete PBR;
    PBR = nullptr;
    Skybox::Deinit();
    Shape::Deinit();

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
    ModelLoader::Deinit();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#else
    ArgumentParser::Deinit();
#endif

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool init()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) 
    {
        spdlog::error("Failed to initalize GLFW!");
        return false;
    }
    
    if (Config::IsVerbose()) spdlog::info("Successfully initialized GLFW!");

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

    if (Config::IsVerbose()) spdlog::info("Successfully created GLFW Window!");

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1); // Enable VSync - fixes FPS at the refresh rate of your screen
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    GLFWimage images[1];
    std::string path = std::string(exeDirPath).append("\\").append(iconPath);

    images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, NULL, 4); //rgba channels

    if (!images[0].pixels) {

        spdlog::error("Failed to load Icon: {}", path);

        stbi_image_free(images[0].pixels);

        return false;
    }
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (err)
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }

    if (Config::IsVerbose()) spdlog::info("Successfully initialized OpenGL loader!");

    // Debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(ErrorMessageCallback, 0);

    if (Config::IsVerbose()) {
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

std::string get_executable_path() {
#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();

#elif defined(__APPLE__)
    char buffer[4096];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::weakly_canonical(buffer).parent_path().string();
    }
    else {
        printf("Error: _NSGetExecutablePath(): Buffer too small for executable path");
        exit(EXIT_FAILURE);
    }

#elif defined(__linux__)
    char buffer[4096];
    ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (count != -1) {
        return std::filesystem::weakly_canonical(std::string(buffer, count)).parent_path().string();
    }
    else {
        printf("Error: readlink(): Cannot read /proc/self/exe");
        exit(EXIT_FAILURE);
    }

#else
#error "Unsupported platform"
#endif
}

void drawBanner()
{
    spdlog::set_pattern("%v");

    spdlog::info("     ___  ___  ___    _   ___               ___              ");
    spdlog::info("    / _ \\/ _ )/ _ \\  | | / (_)__ __ _____ _/ (_)__ ___ ____");
    spdlog::info("   / ___/ _  / , _/  | |/ / (_-</ // / _ `/ / (_-</ -_) __/  ");
    spdlog::info("  /_/  /____/_/|_|   |___/_/___/\\_,_/\\_,_/_/_/___/\\__/_/  \n");
    spdlog::info("                       Version: {}", PBR_VISUALISER_VERSION_STR);
    spdlog::info("             Author: Marceli Antosik (Muppetsg2)");
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
#if WINDOW_APP
    glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
#else
    glClearColor(0.f, 0.f, 0.f, 1.f);
#endif

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
#if WINDOW_APP
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
        case ShapeType::Loaded_Model: {
            if (ModelLoader::IsInit()) {
                if (ModelLoader::HasIndices()) glDrawElements(GL_TRIANGLES, ModelLoader::GetIndicesCount(), GL_UNSIGNED_INT, (void*)ModelLoader::GetIndices());
                else glDrawArrays(GL_TRIANGLES, 0, ModelLoader::GetVerticesCount());
            }
            break;
        }
        default: {
            glDrawElements(GL_TRIANGLES, Shape::GetSphereIndicesCount(), GL_UNSIGNED_INT, (void*)Shape::GetSphereIndices());
        }
    }
#else
    glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
    glBindVertexArray(0);

#if WINDOW_APP
    if (drawSkybox) {
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
        Skybox::Draw();
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
    }
#endif
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

#if WINDOW_APP
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
        case ShapeType::Loaded_Model: {
            if (ModelLoader::IsInit()) {
                glBindBuffer(GL_ARRAY_BUFFER, ModelLoader::GetVBO());

                if (ModelLoader::HasIndices()) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ModelLoader::GetEBO());
                }
                else {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                }
            }
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
    ImGui::StyleColorsNeon();
    ImGui::StyleSizesNeon();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    io.Fonts->Build();
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
    static bool _helpOpen = true;
    static bool _skyboxOpen = true;
    static bool _cameraOpen = true;
    static std::string modelFolderPath = std::string(exeDirPath).append("\\res\\model");
    static std::string screenFolderPath = std::string(exeDirPath).append("\\Screenshots");
    static std::string textureFolderPath = std::string(exeDirPath).append("\\res\\textures");
    static std::string skyboxFolderPath = std::string(exeDirPath).append("\\res\\skybox");
    static std::string lastScreenshotName = "";

    if (!ImGui::Begin("PBR VISUALISER", nullptr, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Menu##Menu")) {
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows##Menu")) {
            if (ImGui::MenuItem("Help", 0, &_helpOpen)) {
                _helpOpen = true;
            }
            if (ImGui::MenuItem("Skybox", 0, &_skyboxOpen)) {
                _skyboxOpen = true;
            }
            if (ImGui::MenuItem("Camera", 0, &_cameraOpen)) {
                _cameraOpen = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::Button("Save Screenshot")) {

        bool exist = false;
        struct stat info;
        if (stat(screenFolderPath.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)) {
            exist = true; // Istnieje i jest folderem
        }

        if (!exist) {
            if (_mkdir(screenFolderPath.c_str()) == 0) {
                spdlog::info("Directory '{}' has been created!", screenFolderPath);
                exist = true;
            }
            else {
                ImGui::OpenPopup("Screenshot Saved Error");
                spdlog::error("Directory '{}' couldn't have been created!", screenFolderPath);
            }
        }

        if (exist) {
            std::pair<bool, std::string> values = Camera::SaveScreenshot(screenFolderPath);
            if (values.first) {
                lastScreenshotName = values.second;
                ImGui::OpenPopup("Screenshot Saved");
            }
            else {
                ImGui::OpenPopup("Screenshot Saved Error");
                spdlog::error("Failed to save screenshot!");
            }
        }
        else {
            ImGui::OpenPopup("Screenshot Saved Error");
            spdlog::error("Failed to save screenshot!");
        }
    }

    ImGui::ShowPopup("Screenshot Saved", ("Screenshot has been saved:\n`" + lastScreenshotName + "`").c_str());
    ImGui::ShowPopup("Screenshot Error", "An error occurred while saving the screenshot.");

    std::string sType = to_string(shapeType);
    std::replace(sType.begin(), sType.end(), '_', ' ');
    bool loadedPopup = false;
    if (ImGui::BeginCombo("Shape", sType.c_str()))
    {
        for (size_t i = 0; i < size<ShapeType>(); ++i) {
            ShapeType acc = (ShapeType)i;
            std::string name = to_string(acc);
            std::replace(name.begin(), name.end(), '_', ' ');
            if (ImGui::Selectable(name.c_str(), shapeType == acc))
            {
                bool change = true;
                if (acc == ShapeType::Loaded_Model && !ModelLoader::IsInit()) {
                    if (!ModelLoader::OpenFileDialog(std::filesystem::exists(modelFolderPath) && std::filesystem::is_directory(modelFolderPath) ? modelFolderPath : exeDirPath)) {
                        change = false;
                    }
                    else {
                        loadedPopup = true;
                    }
                }

                if (change) set_shape(quadVAO, acc);
                break;
            }
        }
        ImGui::EndCombo();
    }

    if (loadedPopup) {
        ImGui::OpenPopup("Model Loaded");
        loadedPopup = false;
    }

    if (ImGui::Button("Upload Model"))
    {
        if (ModelLoader::OpenFileDialog(std::filesystem::exists(modelFolderPath) && std::filesystem::is_directory(modelFolderPath) ? modelFolderPath : exeDirPath)) 
        {
            set_shape(quadVAO, shapeType);
            ImGui::OpenPopup("Model Loaded");
        }
    }

    ImGui::ShowPopup("Model Loaded", ("Model `" + ModelLoader::GetModelName() + "` has been loaded!").c_str());

    ImGui::SameLine();

    std::string n = "File: " + (ModelLoader::GetModelName().empty() ? "Not Loaded" : ModelLoader::GetModelName());
    ImGui::Text(n.c_str());

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

    ImGui::Checkbox("Draw Skybox", &drawSkybox);

    static const ImGuiColorEditFlags config = ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB;
    if (!drawSkybox) ImGui::ColorEdit3("Background Color", (float*)&bgColor, config);

    for (int i = 0; i < 6; ++i)
    {
        ImGui::BeginGroup();
        if (ImGui::Button(("Load Image " + imageName[i]).c_str()))
        {
            std::string openPath = std::filesystem::exists(textureFolderPath) && std::filesystem::is_directory(textureFolderPath) ? textureFolderPath : exeDirPath;

            const char* filters[] = { "*.png", "*.jpg", "*.jpeg" };
            const char* filePath = tinyfd_openFileDialog(
                ("Choose " + imageName[i] + " image").c_str(),
                openPath.c_str(),
                3, filters, "Image Files", 0);

            if (filePath)
            {
                TextureFileFormat inter = i == 0 ? TextureFileFormat::SRGB : i == 1 ? TextureFileFormat::RGB : TextureFileFormat::RED;
                TextureFormat form = i == 0 ? TextureFormat::RGB : i == 1 ? TextureFormat::RGB : TextureFormat::RED;
                imageTextures[i] = new Texture2D(filePath, inter, form);

                if (!imageTextures[i]->IsInit()) {
                    delete imageTextures[i];
                    imageTextures[i] = nullptr;

                    ImGui::OpenPopup(("Error Loading Image " + std::string(imageName[i])).c_str());
                }
            }
        }

        if (ImGui::Button(("Reset##" + imageName[i]).c_str()))
        {
            delete imageTextures[i];
            imageTextures[i] = nullptr;
        }
        ImGui::EndGroup();

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 130);

        if (imageTextures[i] != nullptr)
        {
            ImGui::Image((intptr_t)imageTextures[i]->GetId(), ImVec2(128, 128));
        }
        else
        {
            ImGui::Image((intptr_t)(i >= 2 && i <= 4 ? defaultBlackTexture : defaultWhiteTexture), ImVec2(128, 128));
        }

        ImGui::ShowErrorPopup(("Error Loading Image " + std::string(imageName[i])).c_str(), ("Failed to load " + std::string(imageName[i]) + " image. Please check the file and try again.").c_str());
    }

    ImGui::DragFloat("Height", &height_scale, 0.1f, 0.0f, FLT_MAX);
    ImGui::SliderFloat("Exposure", &exposure, 0.0f, 11.0f, "Exposure: %.2f");
    ImGui::SliderFloat("Color Intensity", &colorIntensity, 0.0f, 4.0f, "Intensity: %.2f");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    drawHelp(&_helpOpen);
    Skybox::DrawEditor(&_skyboxOpen, skyboxFolderPath);
    Camera::DrawEditor(&_cameraOpen);
}

void drawHelp(bool* open)
{
    static Texture2D* icon = nullptr;

    if (!*open) return;

    if (!ImGui::Begin("Help Window", open)) {
        ImGui::End();
        return;
    }

    if (!icon) {
        std::string path = std::string(exeDirPath).append("\\").append(iconPath);
        icon = new Texture2D(path.c_str(), TextureFileFormat::RGBA, TextureFormat::RGBA);
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    float iconSize = 128.f;
    float bannerWidth = ImGui::CalcTextSize("   ___  ___  ___    _   ___               ___            ").x;
    float groupWidth = iconSize + 7.f + bannerWidth;
    float groupX = (windowSize.x - groupWidth) * 0.5f;

    ImGui::SetCursorPosX(groupX);
    if (icon) {
        ImGui::Image((intptr_t)icon->GetId(), ImVec2(iconSize, iconSize));
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(groupX + iconSize + 7.f);
    ImGui::BeginGroup();
    ImGui::Text("   ___  ___  ___    _   ___               ___            ");
    ImGui::Text("  / _ \\/ _ )/ _ \\  | | / (_)__ __ _____ _/ (_)__ ___ ____ ");
    ImGui::Text(" / ___/ _  / , _/  | |/ / (_-</ // / _ `/ / (_-</ -_) __/ ");
    ImGui::Text("/_/  /____/_/|_|   |___/_/___/\\_,_/\\_,_/_/_/___/\\__/_/    ");

    std::string text = std::string("Version: ").append(PBR_VISUALISER_VERSION_STR);
    float textWidth = ImGui::CalcTextSize(text.c_str()).x;
    float textX = (windowSize.x - textWidth + iconSize) * 0.5f;
    ImGui::SetCursorPosX(textX);
    ImGui::Text(text.c_str());
    text = std::string("Author: Marceli Antosik (Muppetsg2)");
    textWidth = ImGui::CalcTextSize(text.c_str()).x;
    textX = (windowSize.x - textWidth + iconSize) * 0.5f;
    ImGui::SetCursorPosX(textX);
    ImGui::Text(text.c_str());
    ImGui::EndGroup();

    ImGui::Separator();
    ImGui::Text("Press 'Left ALT' to toggle between UI cursor mode and Camera movement mode.");
    ImGui::Text("In Camera movement mode, you can look around the scene and move using WSAD.");

    ImGui::Separator();
    ImGui::Text("Use the following keys to move the Camera:");
    ImGui::BulletText("W - Move Forward");
    ImGui::BulletText("S - Move Backward");
    ImGui::BulletText("A - Move Left");
    ImGui::BulletText("D - Move Right");

    ImGui::End();
}


void imgui_end()
{
    ImGui::Render();
    glfwMakeContextCurrent(window);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
#else
// ADDITIONAL FUNCTIONS
bool isValidFileNameWindows(const std::string& name) {
    // Reserved names by Windows
    static const std::vector<std::string> reservedNames = {
        "con", "prn", "aux", "nul", "com1", "com2", "com3", "com4", "com5",
        "com6", "com7", "com8", "com9", "lpt1", "lpt2", "lpt3", "lpt4",
        "lpt5", "lpt6", "lpt7", "lpt8", "lpt9"
    };

    std::string res = name;

    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c)
        {
            return std::tolower(c);
        });

    // Check if name is reserved by Windows
    auto item = std::find(reservedNames.begin(), reservedNames.end(), res);
    if (item != reservedNames.end()) {
        spdlog::warn("Invalid fileName argument: '{}'. The specified file name is a reserved name by Windows.", name);
        return false;
    }

    // Check if name has invalid characters
    std::regex invalidChars(R"([<>:"/\\|?*])");
    if (std::regex_search(name, invalidChars)) {
        spdlog::warn("Invalid fileName argument: '{}'. The specified file name has invalid characters.", name);
        return false;
    }

    // Check if name ends with space or dot
    if (!name.empty() && (name.back() == ' ' || name.back() == '.')) {
        spdlog::warn("Invalid fileName argument: '{}'. The specified file name ends with space or dot.", name);
        return false;
    }

    return true;
}

// PROCESS ARGUMENTS
void printHelp() {

    drawBanner();

    spdlog::set_pattern("%v");

    spdlog::info("Usage:");
    spdlog::info("  PBR_Visualiser.exe ([-h] | [-I] | [-v] [-f <albedo_path> <normal_path> ...] [-n <output_name>] [-d <directory_path>] [-p <position>] [-s <skybox>] [-r <resolution>] [-e <exposure_value>] [-i <color_intensity>])");
    spdlog::info("Options:");
    spdlog::info("  -h                   Display this help message and exit.");
    spdlog::info("  -I                   Turns on the interactive mode where the user will be asked one by one to provide the values needed to generate the image.");
    spdlog::info("                       If the user provides these values through the call arguments, they will be included when generating the first image.");
    spdlog::info("                       After each generated image, the user will be asked whether he wants to generate another image.");
    spdlog::info("  -v                   Throws more detailed information as output to the console");
    spdlog::info("  -f <image_path>      Specify up to 6 image paths to process. Additional paths will be ignored.");
    spdlog::info("                       The paths must be in the order: albedo, normal, metallness, displacement, roughness, ambient occlusion.");
    spdlog::info("                       Example: program -f albedo_image.jpg normal_image.png metalness_image.png");
    spdlog::info("  -n <output_name>     Specify the output file name. Appends '.png' by default. Default is 'PBR_Image'.");
    spdlog::info("  -d <directory_path>  Specify the directory where files will be saved. Default is the current executable path.");
    spdlog::info("  -p <position>        Specify the position of the plane in world. Accepted values: top, bottom, front, back, right, left.");
    spdlog::info("                       Default position is 'back'.");
    spdlog::info("  -s <skybox>          Specify the skybox texture. Accepted values: park, hill, photostudio, bathroom, moonless_golf, snowy_field, venice_sunset, satara_night, golden_bay.");
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
    static const std::vector<std::string> validSkyboxes = { "park", "hill", "photostudio", "bathroom", "moonless_golf", "snowy_field", "venice_sunset", "satara_night", "golden_bay" };

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
        spdlog::warn("Invalid skybox argument: '{}'. Accepted values are: park, hill, photostudio, bathroom, moonless_golf, snowy_field, venice_sunset, satara_night, golden_bay.", arg);
        return;
    }
}

void processNameArgument(const std::string& arg, std::string& fileName) {
    if (!fileName.empty()) {
        spdlog::warn("File name has already been specified! Ignoring additional name.");
        return;
    }

    if (!isValidFileNameWindows(arg)) {
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

// PROCESS INPUT (Interacive Mode Only)
void processFileInput(std::vector<std::string>& imgPaths)
{
    unsigned int i = 0;
    for (const auto& name : imageName) {
        if (imgPaths.size() > i && !imgPaths[i].empty()) {
            spdlog::info("The path to the '{}' map has already been set.", name);
            ++i;
            continue;
        }

        std::string userInput;
        spdlog::info("Enter the '{}' map path [DEFAULT: {}]", name, i >= 2 && i <= 4 ? "Black" : "White");
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        if (imgPaths.size() <= i) {
            imgPaths.push_back(userInput);
        }
        else {
            imgPaths[i] = userInput;
        }

        ++i;
    }
}

void processSkyInput(SkyboxEnum& sky)
{
    static const std::vector<std::string> skyboxes = { "Park", "Hill", "Photostudio", "Bathroom", "Moonless Golf", "Snowy Field", "Venice Sunset", "Satara Night", "Golden Bay" };

    if (sky != SkyboxEnum::DEFAULT) {
        spdlog::info("Skybox was already specified! Selected skybox: {}", to_string(sky));
        return;
    }

    spdlog::info("Choose Skybox:");

    for (size_t i = 0; i < skyboxes.size(); ++i) {
        spdlog::info("{}. {}", i + 1, skyboxes[i]);
    }

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            int choice = std::stoi(userInput);

            if (choice >= 1 && choice <= static_cast<int>(skyboxes.size())) {
                sky = (SkyboxEnum)(uint8_t)(choice - 1);
                spdlog::info("You selected: {}", skyboxes[choice - 1]);
                break;
            }
            else {
                spdlog::error("Invalid choice. Please select a number between 1 and {}.", skyboxes.size());
                continue;
            }
        }
        catch (const std::exception&) {
            spdlog::error("Invalid input. Please enter a valid number.");
            continue;
        }
    }
}

void processNameInput(std::string& fileName)
{
    if (!fileName.empty()) {
        spdlog::info("File name was already specified! Typed name: {}", fileName);
        return;
    }

    spdlog::info("Enter file name");

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        if (!isValidFileNameWindows(userInput)) {
            continue;
        }

        fileName = userInput + ".png";

        spdlog::info("File name: {}", fileName);
        break;
    }
}

void processDirectoryInput(std::string& direc, std::string& last)
{
    if (!direc.empty()) {
        spdlog::info("Save directory was already specified! Typed directory: {}", direc);
        last = direc;
        return;
    }

    if (!last.empty()) {
        spdlog::info("Last used directory: {}", direc);
        spdlog::info("Do you want to use last used directory? [y/n]");

        while (true) {
            std::string userInput;
            std::cout << ">> ";
            std::getline(std::cin, userInput);

            if (userInput == "Y" || userInput == "y") {
                direc = last;
                spdlog::info("Save directory: {}", direc);
                return;
            }
            else if (userInput == "N" || userInput == "n") {
                break;
            }
            else {
                spdlog::info("Unrecognized option. Type 'y' for yes or 'n' for no.");
            }
        }
    }

    spdlog::info("Enter file save directory");

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            std::filesystem::path dirPath = std::filesystem::absolute(userInput);
            if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
                direc = dirPath.string();
                last = dirPath.string();
                spdlog::info("Save directory: {}", direc);
                break;
            }
            else {
                spdlog::info("Provided path is invalid or is not a directory!");
                continue;
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            spdlog::info("Error processing save directory path: {}", e.what());
            continue;
        }
    }
}

void processPositionInput(RenderPosition& position)
{
    static const std::vector<std::string> poses = { "Top", "Bottom", "Front", "Back", "Right", "Left" };

    if (position != RenderPosition::DEFAULT) {
        spdlog::info("Position was already specified! Selected position: {}", to_string(position));
        return;
    }

    spdlog::info("Choose Position:");

    for (size_t i = 0; i < poses.size(); ++i) {
        spdlog::info("{}. {}", i + 1, poses[i]);
    }

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            int choice = std::stoi(userInput);
            if (choice >= 1 && choice <= static_cast<int>(poses.size())) {
                position = (RenderPosition)(uint8_t)(choice - 1);
                spdlog::info("You selected: {}", poses[choice - 1]);
                break;
            }
            else {
                spdlog::error("Invalid choice. Please select a number between 1 and {}.", poses.size());
                continue;
            }
        }
        catch (const std::exception&) {
            spdlog::error("Invalid input. Please enter a valid number.");
            continue;
        }
    }
}

void processResolutionInput(RenderResolution& resolution)
{
    static const std::vector<std::string> reses = { "128x128", "256x256", "512x512", "1024x1024", "2048x2048", "4096x4096" };

    if (resolution != RenderResolution::DEFAULT) {
        spdlog::info("Resolution was already specified! Selected resolution: {}", to_string(resolution));
        return;
    }

    spdlog::info("Choose Resolution:");

    for (size_t i = 0; i < reses.size(); ++i) {
        spdlog::info("{}. {}", i + 1, reses[i]);
    }

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            int choice = std::stoi(userInput);
            if (choice >= 1 && choice <= static_cast<int>(reses.size())) {
                resolution = (RenderResolution)(uint8_t)(choice - 1);
                spdlog::info("You selected: {}", reses[choice - 1]);
                break;
            }
            else {
                spdlog::error("Invalid choice. Please select a number between 1 and {}.", reses.size());
                continue;
            }
        }
        catch (const std::exception&) {
            spdlog::error("Invalid input. Please enter a valid number.");
            continue;
        }
    }
}

void processExposureInput(float& expValue, bool& isValueSet)
{
    const float MIN_EXPOSURE_VALUE = 0.0f;
    const float MAX_EXPOSURE_VALUE = 11.0f;

    if (isValueSet) {
        spdlog::info("Exposure value was already specified and is valid! Typed exposure: {}", expValue);
        return;
    }

    spdlog::info("Enter the exposure value (0.0 - 11.0)");

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            float val = std::stof(userInput);
            if (val >= MIN_EXPOSURE_VALUE && val <= MAX_EXPOSURE_VALUE) {
                expValue = val;
                isValueSet = true;
                spdlog::info("Typed exposure value: {}", val);
                break;
            }
            else {
                spdlog::info("Exposure value out of range ({} - {}): {}", MIN_EXPOSURE_VALUE, MAX_EXPOSURE_VALUE, val);
                continue;
            }
        }
        catch (const std::exception&) {
            spdlog::error("Invalid input. Please enter a valid float number.");
            continue;
        }
    }
}

void processIntensityInput(float& intensityValue, bool& isValueSet)
{
    const float MIN_INTENSITY_VALUE = 0.0f;
    const float MAX_INTENSITY_VALUE = 4.0f;

    if (isValueSet) {
        spdlog::info("Intensity value was already specified and is valid! Typed intensity: {}", intensityValue);
        return;
    }

    spdlog::info("Enter the intensity value (0.0 - 4.0)");

    while (true) {
        std::string userInput;
        std::cout << ">> ";
        std::getline(std::cin, userInput);

        try {
            float val = std::stof(userInput);
            if (val >= MIN_INTENSITY_VALUE && val <= MAX_INTENSITY_VALUE) {
                intensityValue = val;
                isValueSet = true;
                spdlog::info("Typed intensity value: {}", val);
                break;
            }
            else {
                spdlog::info("Intensity value out of range ({} - {}): {}", MIN_INTENSITY_VALUE, MAX_INTENSITY_VALUE, val);
                continue;
            }
        }
        catch (const std::exception&) {
            spdlog::error("Invalid input. Please enter a valid float number.");
            continue;
        }
    }
}

// INTERPRET VALUES
void interpretFileValues(std::vector<std::string>& imgPaths)
{
    std::vector<size_t> err_load = std::vector<size_t>();
    for (size_t i = 0; i < imgPaths.size(); ++i) {
        TextureFileFormat inter = i == 0 ? TextureFileFormat::SRGB : i == 1 ? TextureFileFormat::RGB : TextureFileFormat::RED;
        TextureFormat form = i == 0 ? TextureFormat::RGB : i == 1 ? TextureFormat::RGB : TextureFormat::RED;
        imageTextures[i] = new Texture2D(imgPaths[i].c_str(), inter, form);

        if (!imageTextures[i]->IsInit()) {
            err_load.push_back(i);
            delete imageTextures[i];
            imageTextures[i] = nullptr;
        }
    }

    if (6 - imgPaths.size() > 0) {
        for (size_t z = imgPaths.size(); z < 6; ++z) {
            err_load.push_back(z);
        }
    }

    if (err_load.size() > 0) {
        for (size_t z = 0; z < err_load.size(); ++z) {
            size_t i = err_load[z];
            imageTextures[i] = new Texture2D(i >= 2 && i <= 4 ? defaultBlackTexture : defaultWhiteTexture);
        }
    }

    err_load.clear();
}

std::string interpretSkyValue(SkyboxEnum& sky)
{
    return std::string(exeDirPath + skyboxPaths[(uint8_t)(sky != SkyboxEnum::DEFAULT ? sky : SkyboxEnum::PARK)]);
}

void interpretPositionValue(RenderPosition& position)
{
    trans = glm::mat4(1.f);
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
}

void interpretResolutionValue(RenderResolution& resolution)
{
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
}

// MAIN FUNCTIONS
void generateAndSaveImage(const std::string& fileName, const std::string& saveDir)
{
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
}

void interactiveModeLoop(std::string fileName, std::string saveDir, std::vector<std::string> imgPaths, SkyboxEnum sky, RenderPosition position, RenderResolution resolution)
{
    std::string lastDir;
    while (true) {
        drawBanner();
        spdlog::set_pattern("%v");

        // PROCESS INPUT
        processFileInput(imgPaths);
        processSkyInput(sky);
        processPositionInput(position);
        processExposureInput(exposure, isExposureSet);
        processIntensityInput(colorIntensity, isIntensitySet);
        processNameInput(fileName);
        processDirectoryInput(saveDir, lastDir);
        processResolutionInput(resolution);

        spdlog::set_pattern("%+");
        // INTERPRET VALUES
        interpretFileValues(imgPaths);
        Skybox::ChangeTexture(interpretSkyValue(sky).c_str());
        interpretPositionValue(position);
        interpretResolutionValue(resolution);
        glfwSetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);

        generateAndSaveImage(fileName, saveDir);
        spdlog::set_pattern("%v");

        spdlog::info("Do you want to generate another image? [y/n]");

        while (true) {
            std::string userInput;
            std::cout << ">> ";
            std::getline(std::cin, userInput);

            if (userInput == "Y" || userInput == "y") {
                fileName.clear();
                saveDir.clear();
                imgPaths.clear();
                sky = SkyboxEnum::DEFAULT;
                position = RenderPosition::DEFAULT;
                resolution = RenderResolution::DEFAULT;
                isExposureSet = false;
                isIntensitySet = false;
                break;
            }
            else if (userInput == "N" || userInput == "n") {
                return;
            }
            else {
                spdlog::info("Unrecognized option. Type 'y' for yes or 'n' for no.");
            }
        }
    }
}
#endif