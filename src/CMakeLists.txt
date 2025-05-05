set(PRECOMPILE_HEADERS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
set(CMAKE_AUTORCC ON)

include(${CMAKE_SOURCE_DIR}/cmake/pack_shaders.cmake)

generate_key(6 SHADERS_KEY)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 *.c
	 *.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 *.h
	 *.hpp)
	 
if (WINDOW_APP)
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*(ArgumentParser\.cpp).*")
	list(FILTER HEADER_FILES EXCLUDE REGEX ".*(ArgumentParser\.h).*")
else()
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*(imgui_impl\/|TimeManager\.cpp|ModelLoader\.cpp|CameraController\.cpp).*")
	list(FILTER HEADER_FILES EXCLUDE REGEX ".*(imgui_impl\/|TimeManager\.h|ModelLoader\.h|CameraController\.h).*")
endif()

# Search for the assets files
file(GLOB_RECURSE ASSETS_FILES 
	 ${CMAKE_SOURCE_DIR}/res/*.*)

set_source_files_properties(${ASSETS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Source Files" FILES ${SOURCE_FILES})						   
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Resources"    FILES ${ASSETS_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/icon.png)

# Define the executable
if (WINDOW_APP)
	add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${imgui_FREETYPE_SOURCE} ${imgui_BACKEND_SOURCE} ${ASSETS_FILES} icon.rc)
else()
	add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${ASSETS_FILES} icon.rc)
endif()

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:preprocessor")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG="$<IF:$<CONFIG:Debug>,1,0>")
target_compile_definitions(${PROJECT_NAME} PRIVATE S_KEY="${SHADERS_KEY}")

if(WINDOW_APP)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOW_APP)
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __APPLE__)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __linux__)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC ${PRECOMPILE_HEADERS_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
												  ${glad_SOURCE_DIR}
												  ${stb_image_SOURCE_DIR}
												  ${stb_image_write_SOURCE_DIR})

# Optimizations
target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/RTC1>
    >
    $<$<CXX_COMPILER_ID:GNU>:
        $<$<CONFIG:Release>:-O2>
    >
    $<$<CXX_COMPILER_ID:Clang>:
        $<$<CONFIG:Release>:-O2>
    >
)

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC glad
											 stb_image
											 stb_image_write
											 glfw
											 gli
											 spdlog
											 glm::glm)

if(WINDOW_APP)
	 target_include_directories(${PROJECT_NAME} PUBLIC ${imgui_SOURCE_DIR}
													   ${implot_SOURCE_DIR}
													   ${cgltf_SOURCE_DIR}
													   ${openfbx_SOURCE_DIR}
													   ${tiny_obj_loader_SOURCE_DIR}
													   ${tiny_file_dialogs_SOURCE_DIR})
	 target_link_libraries(${PROJECT_NAME} PUBLIC imgui
												  implot
												  freetype
												  cgltf
												  openfbx
												  tiny_obj_loader
												  tiny_file_dialogs)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC "-framework CoreFoundation")
endif()

# Shader Packing
file(GLOB SHADER_FILES ${CMAKE_SOURCE_DIR}/res/shader/*)

if (WINDOW_APP)
	list(FILTER SHADER_FILES EXCLUDE REGEX ".*(basic2\.vert|basic2\.frag).*")
else()
	list(FILTER SHADER_FILES EXCLUDE REGEX ".*(basic\.vert|basic\.frag|screen\.vert|screen\.frag|skybox\.vert|skybox\.frag).*")
endif()

string(REPLACE ";" " " SHADER_FILES_STR "${SHADER_FILES}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/icon.png
            ${OUTPUT_DIR}/icon.png
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon.png
    COMMENT "Copying icon.png to bin directory"
)

if (WINDOW_APP)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_SOURCE_DIR}/res/build_resources/imgui.ini
				${OUTPUT_DIR}/imgui.ini
		DEPENDS ${CMAKE_SOURCE_DIR}/res/build_resources/imgui.ini
		COMMENT "Copying imgui.ini to bin directory"
	)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_SOURCE_DIR}/res/build_resources/null.txt
			${OUTPUT_DIR}/null.txt
	DEPENDS ${CMAKE_SOURCE_DIR}/res/build_resources/null.txt
	COMMENT "Copying null.txt to bin directory"
)

# Create a separate script to invoke the function
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "cmake_policy(SET CMP0011 NEW)" > ${OUTPUT_DIR}/cmake_post_build.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "include(${CMAKE_SOURCE_DIR}/cmake/pack_shaders.cmake)" >> ${OUTPUT_DIR}/cmake_post_build.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "set(null_path ${OUTPUT_DIR}/null.txt CACHE INTERNAL \\\"Path to null character for shaders\\\")" >> ${OUTPUT_DIR}/cmake_post_build.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "string(REPLACE \\\" \\\" \\\";\\\" SHADER_LIST \\\"\${arg3}\\\")" >> ${OUTPUT_DIR}/cmake_post_build.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "pack_shaders(\\\"\${arg1}\\\" \\\"\${arg2}\\\" \\\"\${SHADER_LIST}\\\")" >> ${OUTPUT_DIR}/cmake_post_build.cmake
    COMMENT "Writing cmake_post_build.cmake script"
)

# Function packing shaders to one binary file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packing Shaders Started..."
    COMMAND ${CMAKE_COMMAND} -E remove -f "${OUTPUT_DIR}/shaders.dat" # Delete old file
    COMMAND ${CMAKE_COMMAND} -E touch "${OUTPUT_DIR}/shaders.dat" # Create new file
    COMMAND ${CMAKE_COMMAND} -Darg1="${SHADERS_KEY}" -Darg2="${OUTPUT_DIR}/shaders.dat" -Darg3="${SHADER_FILES_STR}" -P "${OUTPUT_DIR}/cmake_post_build.cmake"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
				   COMMAND ${CMAKE_COMMAND} -E create_symlink 
				   ${CMAKE_SOURCE_DIR}/res 
				   ${OUTPUT_DIR}/res)