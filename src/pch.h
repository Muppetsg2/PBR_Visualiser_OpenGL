#pragma once

// IMGUI
#include <imgui.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>
#include <imgui_impl/imgui_filedialog.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

// GLAD
#include <glad/glad.h>  // Initialize with gladLoadGL()

// GLFW
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SPDLOG
#include <spdlog/spdlog.h>

// STB_IMAGE
#include <stb_image.h>
#include <stb_image_write.h>

// LIBs
#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cmath>