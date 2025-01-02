#pragma once

#if _DEBUG
// IMGUI
#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <imgui.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>
#include <imgui_impl/imgui_filedialog.h>
#endif

// GLAD
#include <glad/glad.h>  // Initialize with gladLoadGL()

// GLFW
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLI
#include <gli/gli.hpp>

// SPDLOG
#include <spdlog/spdlog.h>

// STB_IMAGE
#include <stb_image.h>
#include <stb_image_write.h>

// LIBs
#include <string>
#include <format>
#include <memory>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

// PROGRAM INFO
#include <version.h>