#pragma once

#if WINDOW_APP
// IMGUI
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>
#include <imgui_impl/imgui_style.h>

// CGLTF
#include <cgltf.h>

// TINY_OBJ_LOADER
#include <tiny_obj_loader.h>

// TINY_FILE_DIALOGS
#include <tinyfiledialogs.h>
#endif

// GLAD
#include <glad/glad.h>  // Initialize with gladLoadGL()

// GLFW
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

// GLM
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
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
#include <algorithm>
#include <cctype>
#include <cmath>
#include <direct.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#if WINDOW_APP
#include <chrono>
#else
#include <iostream>
#endif

// PROGRAM INFO
#include <Config.h>
#include <version.h>