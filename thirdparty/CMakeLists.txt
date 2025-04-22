# CPM - package manager
include(CPM)

# OpenGL
find_package(OpenGL REQUIRED)

# glad https://glad.dav1d.de/
set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)

# stb_image https://github.com/nothings/stb
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image)
set(stb_image_SOURCE_DIR ${STB_IMAGE_DIR} CACHE INTERNAL "")
add_library(stb_image STATIC ${STB_IMAGE_DIR}/stb_image.cpp)
target_include_directories(stb_image PRIVATE ${STB_IMAGE_DIR})

# stb_image_write https://github.com/nothings/stb
set(STB_IMAGE_WRITE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image_write)
set(stb_image_write_SOURCE_DIR ${STB_IMAGE_WRITE_DIR} CACHE INTERNAL "")
add_library(stb_image_write STATIC ${STB_IMAGE_WRITE_DIR}/stb_image_write.cpp)
target_include_directories(stb_image_write PRIVATE ${STB_IMAGE_WRITE_DIR})

# other
CPMAddPackage("gh:glfw/glfw#3.4")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage(
    NAME gli
    GIT_REPOSITORY https://github.com/g-truc/gli.git
    GIT_TAG master
)
CPMAddPackage("gh:gabime/spdlog@1.15.2")

if(WINDOW_APP)
    # cgltf https://github.com/jkuhlmann/cgltf
    set(CGLTF_DIR ${CMAKE_CURRENT_LIST_DIR}/cgltf)
    set(cgltf_SOURCE_DIR ${CGLTF_DIR} CACHE INTERNAL "")
    add_library(cgltf STATIC ${CGLTF_DIR}/cgltf.cpp)
    target_include_directories(cgltf PRIVATE ${CGLTF_DIR})

    # openfbx https://github.com/nem0/OpenFBX
    set(OPENFBX_DIR ${CMAKE_CURRENT_LIST_DIR}/openfbx)
    set(openfbx_SOURCE_DIR ${OPENFBX_DIR} CACHE INTERNAL "")
    add_library(openfbx STATIC ${OPENFBX_DIR}/libdeflate.c ${OPENFBX_DIR}/ofbx.cpp)
    target_include_directories(openfbx PRIVATE ${OPENFBX_DIR})

    # tiny_obj_loader https://github.com/tinyobjloader/tinyobjloader
    set(TINY_OBJ_LOADER_DIR ${CMAKE_CURRENT_LIST_DIR}/tiny_obj_loader)
    set(tiny_obj_loader_SOURCE_DIR ${TINY_OBJ_LOADER_DIR} CACHE INTERNAL "")
    add_library(tiny_obj_loader STATIC ${TINY_OBJ_LOADER_DIR}/tiny_obj_loader.cpp)
    target_include_directories(tiny_obj_loader PRIVATE ${TINY_OBJ_LOADER_DIR})

    # tiny_file_dialogs
    CPMAddPackage(
        NAME tiny_file_dialogs
        GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
        GIT_TAG master
    )

    set(tiny_file_dialogs_SOURCE_DIR ${tiny_file_dialogs_SOURCE_DIR} CACHE INTERNAL "")
    add_library(tiny_file_dialogs STATIC ${tiny_file_dialogs_SOURCE_DIR}/tinyfiledialogs.c)

    CPMAddPackage("gh:freetype/freetype#VER-2-13-3")
    CPMAddPackage("gh:ocornut/imgui@1.91.9b-docking")

    set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "")
    add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
					         ${imgui_SOURCE_DIR}/imgui_demo.cpp
					         ${imgui_SOURCE_DIR}/imgui_draw.cpp
						     ${imgui_SOURCE_DIR}/imgui_tables.cpp
					         ${imgui_SOURCE_DIR}/imgui_widgets.cpp)

    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)
    target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

    set(imgui_FREETYPE_SOURCE ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp CACHE INTERNAL "")
    set(imgui_BACKEND_SOURCE ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp CACHE INTERNAL "")
    
    set_target_properties(imgui
                          freetype
                          cgltf
                          openfbx
                          tiny_obj_loader
                          tiny_file_dialogs PROPERTIES FOLDER "thirdparty")
endif()

set_target_properties(glad
                      stb_image
                      stb_image_write
                      glfw 
                      glm
                      gli
                      spdlog PROPERTIES FOLDER "thirdparty")

if(TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "thirdparty")
endif()
