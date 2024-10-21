# CPM - package manager
include(CPM)

# OpenGL
find_package(OpenGL REQUIRED)

# glad
set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)

# stb_image
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image)
set(stb_image_SOURCE_DIR ${STB_IMAGE_DIR} CACHE INTERNAL "")
add_library(stb_image STATIC ${STB_IMAGE_DIR}/stb_image.cpp)
target_include_directories(stb_image PRIVATE ${STB_IMAGE_DIR})

# stb_image_write
set(STB_IMAGE_WRITE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image_write)
set(stb_image_write_SOURCE_DIR ${STB_IMAGE_WRITE_DIR} CACHE INTERNAL "")
add_library(stb_image_write STATIC ${STB_IMAGE_WRITE_DIR}/stb_image_write.cpp)
target_include_directories(stb_image_write PRIVATE ${STB_IMAGE_WRITE_DIR})

# other
CPMAddPackage("gh:glfw/glfw#3.4")
CPMAddPackage("gh:g-truc/glm#1.0.1")
# CPMAddPackage("gh:g-truc/gli#0.8.2.0")
CPMAddPackage(
  NAME gli
  GIT_REPOSITORY https://github.com/g-truc/gli.git
  GIT_TAG master
)
CPMAddPackage("gh:ocornut/imgui@1.91.3-docking")
CPMAddPackage("gh:gabime/spdlog@1.14.1")

set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "")
add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
					     ${imgui_SOURCE_DIR}/imgui_demo.cpp
					     ${imgui_SOURCE_DIR}/imgui_draw.cpp
						 ${imgui_SOURCE_DIR}/imgui_tables.cpp
					     ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
						 
set_target_properties(glad
                      stb_image
                      stb_image_write
                      glfw 
                      glm
                      gli
                      imgui 
                      spdlog PROPERTIES FOLDER "thirdparty")

if (TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "thirdparty")
endif()
